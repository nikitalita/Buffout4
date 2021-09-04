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
						auto& control = id->strUserEvent;
						const auto trySet = [&](auto& a_device) noexcept {
							if (const auto mouse = event.As<RE::MouseMoveEvent>();
								mouse && (mouse->mouseInputX != 0 || mouse->mouseInputY != 0)) {
								a_device = input;
							} else {
								a_device = input;
							}
						};

						if (_proxied.ui && _proxied.getMenuOpen(*_proxied.ui, _strings.pipboyMenu)) {
							if (control == _strings.look ||
								control == _strings.cursor ||
								control == _strings.leftStick) {
								const auto old = _active.menuing.load();
								trySet(_active.menuing);
								if (old != _active.menuing && _proxied.msgq) {
									_proxied.msgq->AddMessage(_strings.pipboyMenu, RE::UI_MESSAGE_TYPE::kUpdateController);
								}
							}
						}

						if (control == _strings.look) {
							trySet(_active.looking);
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
			[[nodiscard]] bool IsGamepadActiveMenuing() const noexcept { return _active.menuing == Device::gamepad; }

		private:
			using GetMenuOpen_t = bool(RE::UI&, const RE::BSFixedString&);
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
				RE::UIMessageQueue*& msgq{ *reinterpret_cast<RE::UIMessageQueue**>(REL::ID(82123).address()) };
				RE::ControlMap*& controlMap{ *reinterpret_cast<RE::ControlMap**>(REL::ID(325206).address()) };
				GetMenuOpen_t* const getMenuOpen{ reinterpret_cast<GetMenuOpen_t*>(REL::ID(1065114).address()) };
				UpdateGamepadDependentButtonCodes_t* const updateGamepadDependentButtonCodes{ reinterpret_cast<UpdateGamepadDependentButtonCodes_t*>(REL::ID(190238).address()) };
			} _proxied;  // this runs on a per-frame basis, so try to optimize perfomance

			struct
			{
				RE::BSFixedString cursor{ "Cursor" };
				RE::BSFixedString leftStick{ "LeftStick" };
				RE::BSFixedString look{ "Look" };
				RE::BSFixedString pipboyMenu{ RE::PipboyMenu::MENU_NAME };
			} _strings;  // use optimized pointer comparison instead of slow string comparison

			struct
			{
				std::atomic<Device> device{ Device::none };
				std::atomic<Device> looking{ Device::none };
				std::atomic<Device> menuing{ Device::none };
			} _active;
		};

		inline void RefreshCursor(RE::PipboyMenu& a_self)
		{
			bool cursorEnabled = false;
			if (REL::Relocation<std::uint32_t*> curPage{ REL::ID(1287022) }; *curPage == 3) {
				cursorEnabled = !a_self.showingModalMessage;
			}

			const auto handler = DeviceSwapHandler::GetSingleton();
			if (!handler->IsGamepadActiveMenuing()) {
				cursorEnabled = true;
			}

			a_self.UpdateFlag(RE::UI_MENU_FLAGS::kUsesCursor, cursorEnabled);
			if (const auto controls = RE::ControlMap::GetSingleton(); controls) {
				using RE::UserEvents::INPUT_CONTEXT_ID::kLThumbCursor;
				while (controls->PopInputContext(kLThumbCursor)) {}
				if (cursorEnabled) {
					controls->PushInputContext(kLThumbCursor);
				}
			}

			if (cursorEnabled != a_self.pipboyCursorEnabled) {
				if (const auto ui = RE::UI::GetSingleton(); ui) {
					ui->RefreshCursor();
				}

				if (cursorEnabled && handler->IsGamepadActiveMenuing()) {
					if (const auto cursor = RE::MenuCursor::GetSingleton(); cursor) {
						cursor->CenterCursor();
					}
				}
			}

			a_self.pipboyCursorEnabled = cursorEnabled;
		}

		inline void InstallRefreshCursorPatch()
		{
			const auto target = REL::ID(1533778).address();
			stl::asm_jump(target, 0xE9, reinterpret_cast<std::uintptr_t>(RefreshCursor));
		}

		inline void DisableDisconnectHandler()
		{
			REL::ID target{ 548136 };
			REL::safe_fill(
				target.address() + 0x98,
				REL::NOP,
				0x4E);
		}

		inline void DisableKBMIgnore()
		{
			const auto target = REL::ID(647956).address();
			REL::safe_fill(target + 0x39, REL::NOP, 0x6);
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
			patch(REL::ID(643948), 0x583);  // PipboyMenu::ProcessMessage
		}
	}

	inline void PreLoad()
	{
		detail::DisableDisconnectHandler();
		detail::DisableKBMIgnore();
		detail::InstallRefreshCursorPatch();
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
