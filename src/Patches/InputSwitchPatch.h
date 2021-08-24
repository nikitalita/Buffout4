#pragma once

namespace Patches::InputSwitchPatch
{
	namespace detail
	{
		class DeviceSwapHandler :
			public RE::BSInputEventUser
		{
		public:
			[[nodiscard]] static DeviceSwapHandler* GetSingleton()
			{
				static DeviceSwapHandler singleton;
				return &singleton;
			}

			DeviceSwapHandler(const DeviceSwapHandler&) = delete;
			DeviceSwapHandler& operator=(const DeviceSwapHandler&) = delete;

			bool ShouldHandleEvent(const RE::InputEvent* a_event) override
			{
				if (a_event) {
					const auto& event = *a_event;
					auto input = Device::none;
					switch (*event.device) {
					case RE::INPUT_DEVICE::kKeyboard:
					case RE::INPUT_DEVICE::kMouse:
						input = Device::kbm;
						break;
					case RE::INPUT_DEVICE::kGamepad:
						input = Device::gamepad;
						break;
					}

					if (input != Device::none && _active.device != input) {
						_active.device = input;
						UpdateControls();
					}

					if (const auto id = event.As<RE::IDEvent>(); id) {
						const auto& control = id->strUserEvent;
						if (control == _strings.forward ||
							control == _strings.back ||
							control == _strings.strafeLeft ||
							control == _strings.strafeRight ||
							control == _strings.move) {
							_active.moving = input;
						} else if (control == _strings.look) {
							if (const auto mouse = event.As<RE::MouseMoveEvent>(); mouse) {
								if (mouse->mouseInputX != 0 || mouse->mouseInputY != 0) {
									_active.looking = input;
								}
							} else {
								_active.looking = input;
							}
						}
					}

					if (_proxied.controlMap) {
						_proxied.controlMap->SetIgnoreKeyboardMouse(false);
					}
				}

				return false;
			}

			[[nodiscard]] bool IsGamepadActiveDevice() const noexcept { return _active.device == Device::gamepad; }
			[[nodiscard]] bool IsGamepadActiveLooking() const noexcept { return _active.looking == Device::gamepad; }
			[[nodiscard]] bool IsGamepadActiveMoving() const noexcept { return _active.moving == Device::gamepad; }

		private:
			using UpdateGamepadDependentButtonCodes_t = void(bool);

			DeviceSwapHandler() = default;

			enum class Device
			{
				none,
				kbm,
				gamepad
			};

			void UpdateControls()
			{
				if (_proxied.ui) {
					_proxied.ui->UpdateControllerType();
				}

				_proxied.updateGamepadDependentButtonCodes(IsGamepadActiveDevice());
			}

			struct
			{
				RE::UI*& ui{ *reinterpret_cast<RE::UI**>(REL::ID(548587).address()) };
				RE::ControlMap*& controlMap{ *reinterpret_cast<RE::ControlMap**>(REL::ID(325206).address()) };
				UpdateGamepadDependentButtonCodes_t* const updateGamepadDependentButtonCodes{ reinterpret_cast<UpdateGamepadDependentButtonCodes_t*>(REL::ID(190238).address()) };
			} _proxied;  // this runs on a per-frame basis, so try to optimize perfomance

			struct
			{
				RE::BSFixedString forward{ "Forward" };
				RE::BSFixedString back{ "Back" };
				RE::BSFixedString strafeLeft{ "StrafeLeft" };
				RE::BSFixedString strafeRight{ "StrafeRight" };
				RE::BSFixedString move{ "Move" };
				RE::BSFixedString look{ "Look" };
			} _strings;  // use optimized pointer comparison instead of slow string comparison

			struct
			{
				std::atomic<Device> device{ Device::none };
				std::atomic<Device> moving{ Device::none };
				std::atomic<Device> looking{ Device::none };
			} _active;
		};

		inline void DisableDisconnectHandler()
		{
			REL::ID target{ 548136 };
			REL::safe_fill(
				target.address() + 0x98,
				REL::NOP,
				0x4E);
		}

		inline bool IsGamepadConnected(const RE::BSInputDeviceManager&)
		{
			const auto handler = DeviceSwapHandler::GetSingleton();
			return handler->IsGamepadActiveDevice();
		}

		inline void InstallGamepadConnectedPatch()
		{
			const auto target = REL::ID(609928).address();
			constexpr std::size_t size = 0x25;
			REL::safe_fill(target, REL::INT3, size);
			stl::asm_jump(target, size, reinterpret_cast<std::uintptr_t>(IsGamepadConnected));
		}

		inline bool UsingGamepad(const RE::BSInputDeviceManager&)
		{
			const auto handler = DeviceSwapHandler::GetSingleton();
			return handler->IsGamepadActiveDevice();
		}

		inline void InstallUsingGamepadPatch()
		{
			const auto target = REL::ID(875683).address();
			constexpr std::size_t size = 0x25;
			REL::safe_fill(target, REL::INT3, size);
			stl::asm_jump(target, size, reinterpret_cast<std::uintptr_t>(UsingGamepad));
		}

		inline bool UsingGamepadLook(const RE::BSInputDeviceManager*)
		{
			const auto handler = DeviceSwapHandler::GetSingleton();
			return handler->IsGamepadActiveLooking();
		}

		inline void InstallGamepadLookPatches()
		{
			const auto patch = [](REL::ID a_base, std::size_t a_offset) {
				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_call<5>(a_base.address() + a_offset, UsingGamepadLook);
			};

			patch(REL::ID(1349441), 0x58);  // LevelUpMenu::ZoomGrid
			patch(REL::ID(455462), 0x43);   // PlayerControls::ProcessLookInput
			patch(REL::ID(53721), 0x56);    // PlayerControlsUtils::ProcessLookControls
			patch(REL::ID(1262531), 0x1F);  // FirstPersonState::CalculatePitchOffsetChaseValue
		}
	}

	inline void PreLoad()
	{
		detail::DisableDisconnectHandler();
		detail::InstallGamepadConnectedPatch();
		detail::InstallUsingGamepadPatch();
		detail::InstallGamepadLookPatches();
		logger::debug("installed InputSwitch pre-patch"sv);
	}

	inline void PostInit()
	{
		if (const auto controls = RE::MenuControls::GetSingleton(); controls) {
			controls->handlers.insert(
				controls->handlers.begin(),
				detail::DeviceSwapHandler::GetSingleton());
		}

		logger::debug("installed InputSwitch post-patch"sv);
	}
}
