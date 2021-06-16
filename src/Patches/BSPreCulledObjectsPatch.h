namespace Patches::BSPreCulledObjectsPatch
{
	namespace detail
	{
		class IDTo3DHandler;

		[[nodiscard]] inline auto SplitID(std::uint32_t a_id) noexcept
			-> std::pair<
				std::uint32_t,  // form id w/o file index
				std::uint8_t>   // file index
		{
			return {
				a_id & 0x00FFFFFF,
				static_cast<std::uint8_t>((a_id >> (8 * 3)) & 0xFF)
			};
		}

		class IDTo3DMap
		{
		public:
			using value_type =
				robin_hood::pair<
					std::uint32_t,                   // full form id
					RE::NiPointer<RE::NiAVObject>>;  // object 3D
			using container_type =
				std::vector<
					value_type,
					tbb::scalable_allocator<value_type>>;

			[[nodiscard]] auto find(std::uint32_t a_id) const noexcept
				-> std::span<const value_type>
			{
				const auto [base, idx] = SplitID(a_id);
				if (idx == 0x00 || idx == 0xFD) [[likely]] {
					const auto it = _special.find(a_id);
					if (it != _special.end()) {
						return { std::addressof(*it), 1 };
					}
				} else {
					const auto it = _generic.find(base);
					if (it != _generic.end()) {
						return { it->second.begin(), it->second.end() };
					}
				}

				return {};
			}

			void insert_or_assign(std::uint32_t a_id, RE::NiAVObject* a_obj) noexcept
			{
				const auto idx = SplitID(a_id).second;
				if (idx == 0x00 || idx == 0xFD) [[likely]] {
					insert_special(a_id, a_obj);
				} else {
					insert_generic(a_id, a_obj);
				}
			}

			void erase(std::uint32_t a_id) noexcept
			{
				const auto idx = SplitID(a_id).second;
				if (idx == 0x00 || idx == 0xFD) [[likely]] {
					erase_special(a_id);
				} else {
					erase_generic(a_id);
				}
			}

		protected:
			friend class IDTo3DHandler;

			IDTo3DMap() = default;
			IDTo3DMap(const IDTo3DMap&) = default;
			IDTo3DMap(IDTo3DMap&&) = default;
			~IDTo3DMap() = default;
			IDTo3DMap& operator=(const IDTo3DMap&) = default;
			IDTo3DMap& operator=(IDTo3DMap&&) = default;

		private:
			struct equal_t
			{
				[[nodiscard]] bool operator()(const value_type& a_value) noexcept
				{
					return a_value.first == val;
				}

				const std::uint32_t val;
			};

			struct less_t
			{
				[[nodiscard]] bool operator()(const value_type& a_lhs, const value_type& a_rhs) noexcept
				{
					return a_lhs.first < a_rhs.first;
				}

				[[nodiscard]] bool operator()(const value_type& a_lhs, std::uint32_t a_rhs) noexcept
				{
					return a_lhs.first < a_rhs;
				}
			};

			static void fixup_container(container_type& a_container) noexcept
			{
				// vanilla iterates from lowest to highest index
				std::sort(a_container.begin(), a_container.end(), less_t{});
			}

			void erase_generic(std::uint32_t a_id) noexcept
			{
				const auto [base, idx] = SplitID(a_id);
				const auto it = _generic.find(base);
				if (it != _generic.end()) [[likely]] {
					auto& objs = it->second;
					const auto rm = std::lower_bound(objs.begin(), objs.end(), a_id, less_t{});
					if (rm != objs.end() && rm->first == a_id) {
						assert(std::count_if(objs.begin(), objs.end(), equal_t{ a_id }) == 1);
						objs.erase(rm);
						if (objs.empty()) {
							_generic.erase(it);
						}
					}
				}
			}

			void erase_special(std::uint32_t a_id) noexcept { _special.erase(a_id); }

			void insert_generic(std::uint32_t a_id, RE::NiAVObject* a_obj)
			{
				const auto base = SplitID(a_id).first;
				auto& objs = [&]() -> container_type& {
					const auto it = _generic.find(base);
					if (it != _generic.end()) {
						return it->second;
					} else {
						return _generic
						    .emplace(
								base,
								std::initializer_list<value_type>{})
						    .first->second;
					}
				}();

				if (const auto it = std::lower_bound(objs.begin(), objs.end(), a_id, less_t{});
					it != objs.end() && it->first == a_id) [[unlikely]] {
					assert(std::count_if(objs.begin(), objs.end(), equal_t{ a_id }) == 1);
					it->second.reset(a_obj);
				} else {
					objs.emplace_back(a_id, a_obj);
				}

				fixup_container(objs);
			}

			void insert_special(std::uint32_t a_id, RE::NiAVObject* a_obj)
			{
				const auto it = _special.find(a_id);
				if (it != _special.end()) {
					it->second.reset(a_obj);
				} else {
					_special.emplace(a_id, a_obj);
				}
			}

			robin_hood::unordered_flat_map<
				std::uint32_t,  // form id w/o file index
				container_type>
				_generic;
			robin_hood::unordered_flat_map<
				std::uint32_t,  // full form id
				value_type::second_type>
				_special;
		};

		class IDTo3DHandler
		{
		public:
			using value_type = IDTo3DMap;
			using lock_type = std::mutex;

			class Accessor
			{
			public:
				[[nodiscard]] value_type& operator*() noexcept { return _proxy; }
				[[nodiscard]] const value_type& operator*() const noexcept { return _proxy; }
				[[nodiscard]] value_type* operator->() noexcept { return std::addressof(_proxy); }
				[[nodiscard]] const value_type* operator->() const noexcept { return std::addressof(_proxy); }

			protected:
				friend class IDTo3DHandler;

				Accessor(lock_type& a_lock, value_type& a_proxy) noexcept :
					_locker(a_lock),
					_proxy(a_proxy)
				{}

			private:
				std::lock_guard<lock_type> _locker;
				value_type& _proxy;
			};

			IDTo3DHandler(const IDTo3DHandler&) = delete;
			IDTo3DHandler(IDTo3DHandler&&) = delete;
			IDTo3DHandler& operator=(const IDTo3DHandler&) = delete;
			IDTo3DHandler& operator=(IDTo3DHandler&&) = delete;

			[[nodiscard]] static IDTo3DHandler& GetSingleton() noexcept
			{
				static IDTo3DHandler singleton;
				return singleton;
			}

			[[nodiscard]] Accessor Access() noexcept { return { _lock, _value }; }

			// BSCullingGroup::Add
			void AddToCullingGroup(RE::BSCullingGroup& a_self, RE::NiAVObject* a_obj, const RE::NiBound& a_bound, std::uint32_t a_flags) const
			{
				return _addToCullingGroup(a_self, a_obj, a_bound, a_flags);
			}

			// BSPreCulledObjects::CallVisibilityCallbacks
			void CallVisibilityCallbacks() const { return _callVisibilityCallbacks(); }

			// BSPreCulledObjects::pPreCulledDynamicObjectsA
			[[nodiscard]] auto PreCulledDynamicObjects() const noexcept { return _preCulledDynamicObjects; }

			// BSPreCulledObjects::pPreCulledDynamicRainObjectsA
			[[nodiscard]] auto PreCulledDynamicRainObjects() const noexcept { return _preCulledDynamicRainObjects; }

			// BSPreCulledObjects::pPreCulledDynamicShadowObjectsA
			[[nodiscard]] auto PreCulledDynamicShadowObjects() const noexcept { return _preCulledDynamicShadowObjects; }

			// BSPreCulledObjects::PreCulledIDA
			[[nodiscard]] decltype(auto) PreCulledIDs() const noexcept { return _preCulledIDs; }

			// BSPreCulledObjects::PreCulledRainIDA
			[[nodiscard]] decltype(auto) PreCulledRainIDs() const noexcept { return _preCulledRainIDs; }

			// BSPreCulledObjects::PreCulledShadowIDA
			[[nodiscard]] decltype(auto) PreCulledShadowIDs() const noexcept { return _preCulledShadowIDs; }

			// BSPreCulledObjects::RemoveVisibilityCallBackForID
			[[nodiscard]] void RemoveVisibilityCallBackForID(std::uint32_t a_id) const
			{
				return _removeVisibilityCallBackForID(a_id);
			}

			// BSPreCulledObjects::TrackVisibility
			void TrackVisibility(std::uint32_t a_id) const
			{
				return _trackVisibility(a_id);
			}

			// BSPreCulledObjects::bUsePreCulledObjects
			[[nodiscard]] bool UsePreCulledObjects() const noexcept { return _usePreCulledObjects.GetBinary(); }

		private:
			using AddToCullingGroup_t = void(RE::BSCullingGroup&, RE::NiAVObject*, const RE::NiBound&, std::uint32_t);
			using CallVisibilityCallbacks_t = void();
			using RemoveVisibilityCallBackForID_t = void(std::uint32_t);
			using TrackVisibility_t = void(std::uint32_t);

			using PreCulledObjects_t = RE::BSTObjectArena<RE::BSPreCulledObjects::ObjectRecord, RE::BSTObjectArenaScrapAlloc, 512>;
			using PreCulledIDs_t = RE::BSTArray<std::uint32_t>;

			IDTo3DHandler() = default;
			~IDTo3DHandler() = default;

			AddToCullingGroup_t* const _addToCullingGroup{ reinterpret_cast<AddToCullingGroup_t*>(REL::ID(1175493).address()) };
			CallVisibilityCallbacks_t* const _callVisibilityCallbacks{ reinterpret_cast<CallVisibilityCallbacks_t*>(REL::ID(887475).address()) };
			RemoveVisibilityCallBackForID_t* const _removeVisibilityCallBackForID{ reinterpret_cast<RemoveVisibilityCallBackForID_t*>(REL::ID(136788).address()) };
			TrackVisibility_t* const _trackVisibility{ reinterpret_cast<TrackVisibility_t*>(REL::ID(1167260).address()) };

			const PreCulledObjects_t*& _preCulledDynamicObjects{ *reinterpret_cast<const PreCulledObjects_t**>(REL::ID(713154).address()) };
			const PreCulledObjects_t*& _preCulledDynamicRainObjects{ *reinterpret_cast<const PreCulledObjects_t**>(REL::ID(274004).address()) };
			const PreCulledObjects_t*& _preCulledDynamicShadowObjects{ *reinterpret_cast<const PreCulledObjects_t**>(REL::ID(932019).address()) };

			const PreCulledIDs_t& _preCulledIDs{ *reinterpret_cast<PreCulledIDs_t*>(REL::ID(1370583).address()) };
			const PreCulledIDs_t& _preCulledRainIDs{ *reinterpret_cast<PreCulledIDs_t*>(REL::ID(1104804).address()) };
			const PreCulledIDs_t& _preCulledShadowIDs{ *reinterpret_cast<PreCulledIDs_t*>(REL::ID(446402).address()) };

			const RE::Setting& _usePreCulledObjects{ *reinterpret_cast<RE::Setting*>(REL::ID(1252712).address()) };

			lock_type _lock;    // replaces BSPreCulledObjects::IDMapLock
			value_type _value;  // replaces BSPreCulledObjects::IDto3Dmap
		};

		template <class IDCallback, class ObjectCallback>
		__forceinline void DoAddToCullingGroup(
			const RE::BSTArray<std::uint32_t>& a_ids,
			IDCallback a_idFn,
			const RE::BSTObjectArena<RE::BSPreCulledObjects::ObjectRecord, RE::BSTObjectArenaScrapAlloc, 512>* a_objects,
			ObjectCallback a_objFn)  //
			requires((
				std::invocable<IDCallback, RE::NiAVObject&, std::uint32_t> &&
				std::invocable<ObjectCallback, const RE::BSPreCulledObjects::ObjectRecord&>))
		{
			{
				const auto token = IDTo3DHandler::GetSingleton().Access();
				for (const auto needle : a_ids) {
					for (const auto& [full, obj] : token->find(needle)) {
						assert(obj != nullptr);
						a_idFn(*obj, full);
					}
				}
			}

			if (a_objects) {
				for (const auto& record : *a_objects) {
					a_objFn(record);
				}
			}
		}

		inline void AddToCullingGroup(RE::BSCullingGroup& a_cullingGroup, RE::BSCullingGroup& a_nonShadowCullingGroup)
		{
			auto& handler = IDTo3DHandler::GetSingleton();
			DoAddToCullingGroup(
				handler.PreCulledIDs(),
				[&](RE::NiAVObject& a_obj, std::uint32_t a_id) {
					if (!a_obj.GetAppCulled()) {
						auto& group = a_obj.ShadowCaster() ? a_cullingGroup : a_nonShadowCullingGroup;
						handler.AddToCullingGroup(group, std::addressof(a_obj), a_obj.worldBound, 0);
						handler.TrackVisibility(a_id);
					}
				},
				handler.PreCulledDynamicObjects(),
				[&](const RE::BSPreCulledObjects::ObjectRecord& a_record) {
					if (a_record.obj->ShadowCaster()) {
						handler.AddToCullingGroup(
							a_cullingGroup,
							a_record.obj,
							a_record.obj->worldBound,
							a_record.flags);
					}
				});
			handler.CallVisibilityCallbacks();
		}

		inline void AddToRainCullingGroup(RE::BSCullingGroup& a_cullingGroup)
		{
			auto& handler = IDTo3DHandler::GetSingleton();
			DoAddToCullingGroup(
				handler.PreCulledRainIDs(),
				[&](RE::NiAVObject& a_obj, std::uint32_t) {
					if (!a_obj.GetAppCulled()) {
						handler.AddToCullingGroup(a_cullingGroup, std::addressof(a_obj), a_obj.worldBound, 0);
					}
				},
				handler.PreCulledDynamicRainObjects(),
				[&](const RE::BSPreCulledObjects::ObjectRecord& a_record) {
					handler.AddToCullingGroup(
						a_cullingGroup,
						a_record.obj,
						a_record.obj->worldBound,
						a_record.flags);
				});
		}

		inline void AddToShadowCullingGroup(RE::BSCullingGroup& a_cullingGroup)
		{
			auto& handler = IDTo3DHandler::GetSingleton();
			DoAddToCullingGroup(
				handler.PreCulledShadowIDs(),
				[&](RE::NiAVObject& a_obj, std::uint32_t) {
					// !a_obj.GetAppCulled() && a_obj.ShadowCaster()
					if ((a_obj.GetFlags() & 0x100'0000'0001) == 0) {
						handler.AddToCullingGroup(a_cullingGroup, std::addressof(a_obj), a_obj.worldBound, 0);
					}
				},
				handler.PreCulledDynamicShadowObjects(),
				[&](const RE::BSPreCulledObjects::ObjectRecord& a_record) {
					if (a_record.obj->ShadowCaster()) {
						handler.AddToCullingGroup(
							a_cullingGroup,
							a_record.obj,
							a_record.obj->worldBound,
							a_record.flags);
					}
				});
		}

		inline void UpdateIDto3DMap(std::uint32_t a_id, RE::NiAVObject* a_obj)
		{
			auto& handler = IDTo3DHandler::GetSingleton();
			if (handler.UsePreCulledObjects()) {
				auto token = handler.Access();
				if (a_obj) {
					token->insert_or_assign(a_id, a_obj);
				} else {
					token->erase(a_id);
					handler.RemoveVisibilityCallBackForID(a_id);
				}
			}
		}

		inline void WriteAddToCullingGroup()
		{
			const auto target = REL::ID(997287).address();
			constexpr auto size = 0x278;
			REL::safe_fill(target, REL::INT3, size);
			stl::asm_jump(target, size, reinterpret_cast<std::uintptr_t>(AddToCullingGroup));
		}

		inline void WriteAddToRainCullingGroup()
		{
			const auto target = REL::ID(410310).address();
			constexpr auto size = 0x218;
			REL::safe_fill(target, REL::INT3, size);
			stl::asm_jump(target, size, reinterpret_cast<std::uintptr_t>(AddToRainCullingGroup));
		}

		inline void WriteAddToShadowCullingGroup()
		{
			const auto target = REL::ID(1142692).address();
			constexpr auto size = 0x259;
			REL::safe_fill(target, REL::INT3, size);
			stl::asm_jump(target, size, reinterpret_cast<std::uintptr_t>(AddToShadowCullingGroup));
		}

		inline void WriteUpdateIDto3DMap()
		{
			const auto target = REL::ID(1162647).address();
			constexpr auto size = 0x1AA;
			REL::safe_fill(target, REL::INT3, size);
			stl::asm_jump(target, size, reinterpret_cast<std::uintptr_t>(UpdateIDto3DMap));
		}
	}

	inline void Install()
	{
		detail::WriteAddToCullingGroup();
		detail::WriteAddToRainCullingGroup();
		detail::WriteAddToShadowCullingGroup();
		detail::WriteUpdateIDto3DMap();

		logger::debug("installed BSPreCulledObjects patch"sv);
	}
}
