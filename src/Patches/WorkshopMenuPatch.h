#pragma once

namespace Patches::WorkshopMenuPatch
{
	namespace detail
	{
		class CompareFactory
		{
		private:
			struct eq_t
			{
				template <class T>
				[[nodiscard]] bool operator()(
					const T& a_lhs,
					const T& a_rhs) const noexcept  //
					requires(std::same_as<std::remove_cv_t<typename std::pointer_traits<T>::element_type>, RE::Workshop::WorkshopMenuNode>)
				{
					assert(a_lhs && a_rhs);
					return a_lhs->filterKeyword == a_rhs->filterKeyword &&
					       a_lhs->recipe == a_rhs->recipe &&
					       a_lhs->sourceFormListRecipe == a_rhs->sourceFormListRecipe &&
					       a_lhs->form == a_rhs->form;
				}
			};

			struct less_t
			{
				template <class T>
				[[nodiscard]] bool operator()(
					const T& a_lhs,
					const T& a_rhs) const noexcept  //
					requires(std::same_as<std::remove_cv_t<typename std::pointer_traits<T>::element_type>, RE::Workshop::WorkshopMenuNode>)
				{
					assert(a_lhs && a_rhs);

					const auto [lCobj, rCobj] =
						a_lhs->sourceFormListRecipe && a_rhs->sourceFormListRecipe ?
                            std::make_pair(a_lhs->sourceFormListRecipe, a_rhs->sourceFormListRecipe) :
                            std::make_pair(a_lhs->recipe, a_rhs->recipe);
					if (!lCobj || !rCobj) {
						return false;
					}

					const auto compNames = [&](const RE::BGSConstructibleObject& a_lhs, const RE::BGSConstructibleObject& a_rhs) {
						const auto getName = [&](const RE::BGSConstructibleObject& a_cobj) {
							const auto created = a_cobj.GetCreatedItem();
							if (const auto it = created ? _names.find(created) : _names.end(); it != _names.end()) {
								return static_cast<std::string_view>(it->second);
							} else {
								return ""sv;
							}
						};

						const auto lName = getName(a_lhs);
						const auto rName = getName(a_rhs);
						return lName.compare(rName);
					};

					if (const auto lPrio = lCobj->GetWorkshopPriority(), rPrio = rCobj->GetWorkshopPriority();
						lPrio != rPrio) {
						return lPrio < rPrio;
					} else if (const auto naming = compNames(*lCobj, *rCobj);
							   naming != 0) {
						return naming < 0;
					} else {
						return lCobj->GetFormID() != rCobj->GetFormID() ?
                                   lCobj->GetFormID() < rCobj->GetFormID() :
                                   a_lhs->form &&
						               a_rhs->form &&
						               a_lhs->form->GetFormID() < a_rhs->form->GetFormID();
					}
				}

				const RE::BSTHashMap<const RE::TESForm*, RE::BGSLocalizedString>& _names;
			};

		public:
			[[nodiscard]] eq_t equal_to() const noexcept { return {}; }
			[[nodiscard]] less_t less() const noexcept { return { _names }; }

		public:
			const RE::BSTHashMap<const RE::TESForm*, RE::BGSLocalizedString>& _names{ RE::TESFullName::GetSparseFullNameMap() };
		};

		class NodeFactory
		{
		public:
			NodeFactory(RE::Workshop::WorkshopMenuNode& a_parent, std::uint32_t& a_uniqueID) noexcept :
				_parent(a_parent),
				_uniqueID(a_uniqueID)
			{}

			NodeFactory(const NodeFactory&) = default;
			NodeFactory(NodeFactory&&) = default;
			~NodeFactory() = default;
			NodeFactory& operator=(const NodeFactory&) = default;
			NodeFactory& operator=(NodeFactory&&) = default;

			[[nodiscard]] RE::Workshop::WorkshopMenuNode& operator()()
			{
				const auto mem = _allocate(_mm, sizeof(RE::Workshop::WorkshopMenuNode), 0, false);
				assert(mem != nullptr);
				const auto& child =
					_parent.children.emplace_back(
						std::construct_at(
							std::launder(static_cast<RE::Workshop::WorkshopMenuNode*>(mem))));
				child->parent = &_parent;
				child->uniqueID = ++_uniqueID;
				child->row = _parent.row + 1;
				return *child;
			}

			[[nodiscard]] NodeFactory clone(RE::Workshop::WorkshopMenuNode& a_parent) const noexcept
			{
				return { a_parent, _uniqueID };
			}

		private:
			using Allocate_t = void*(RE::MemoryManager&, std::size_t, std::uint32_t, bool);

			RE::Workshop::WorkshopMenuNode& _parent;
			std::uint32_t& _uniqueID;
			RE::MemoryManager& _mm{ RE::MemoryManager::GetSingleton() };
			Allocate_t* _allocate{ reinterpret_cast<Allocate_t*>(REL::ID(652767).address()) };
		};

		class LookupTable
		{
		public:
			using key_type = RE::BGSKeyword*;
			using mapped_type = std::vector<std::reference_wrapper<RE::BGSConstructibleObject>>;
			using value_type = std::pair<const key_type, mapped_type>;

			LookupTable() { assert(_currentWorkshop != nullptr); }

			[[nodiscard]] const mapped_type& operator()(RE::BGSKeyword& a_key)
			{
				const auto key = &a_key != _badKeyword ? &a_key : nullptr;
				if (const auto it = _lookupCache.find(key); it != _lookupCache.end()) {
					return it->second;
				} else if (_currentWorkshop) {
					mapped_type results;
					for (const auto cobj : _cobjs) {
						if (WorkshopCanShowRecipe(cobj, key)) {
							results.push_back(cobj);
						}
					}
					const auto ins = _lookupCache.emplace(key, std::move(results));
					return ins.first->second;
				} else [[unlikely]] {
					const auto ins = _lookupCache.emplace(key, mapped_type());
					return ins.first->second;
				}
			}

		private:
			using IsTrueForAllButFunction_t = bool(const RE::TESCondition&, RE::ConditionCheckParams&, RE::SCRIPT_OUTPUT);

			template <class Key, class T, template <class> class Allocator>
			using map_t = std::map<Key, T, std::less<Key>, Allocator<std::pair<const Key, T>>>;

			template <class C, class T>
			[[nodiscard]] __forceinline static bool BinarySearch(const C& a_haystack, T&& a_needle)
			{
				const auto it = std::lower_bound(a_haystack.begin(), a_haystack.end(), a_needle);
				return it != a_haystack.end() && *it == a_needle;
			}

			template <class C, class T>
			[[nodiscard]] __forceinline static bool LinearSearch(const C& a_haystack, T&& a_needle)
			{
				return std::find(a_haystack.begin(), a_haystack.end(), a_needle) != a_haystack.end();
			}

			[[nodiscard]] bool HasKeyword(const RE::BGSKeyword* a_keyword)
			{
				assert(_currentWorkshop != nullptr);

				if (const auto it = _hasKeywordCache.find(a_keyword); it != _hasKeywordCache.end()) {
					return it->second;
				} else {
					const auto ins = _hasKeywordCache.emplace(
						a_keyword,
						_currentWorkshop->HasKeyword(a_keyword, _currentInstance.get()));
					return ins.first->second;
				}
			}

			[[nodiscard]] __forceinline bool HasStoredItem(const RE::TESForm& a_item) const
			{
				const auto needle =
					&a_item == _splineEndpointMarker && _workshopSplineObject ?
                        _workshopSplineObject->GetFormID() :
                        a_item.GetFormID();
				return BinarySearch(_storedItems, needle);
			}

			[[nodiscard]] bool WorkshopCanShowRecipe(RE::BGSConstructibleObject& a_recipe, RE::BGSKeyword* a_filter)
			{
				assert(_currentWorkshop != nullptr);
				assert(a_recipe.createdItem != nullptr);
				assert(a_recipe.createdItem->IsObject() || a_recipe.createdItem->Is<RE::BGSListForm>());

				_canShowCache.clear();
				for (std::uint32_t i = 0; i < a_recipe.filterKeywords.size; ++i) {
					const auto index = a_recipe.filterKeywords.array[i].keywordIndex;
					if (index < _filters.size()) {
						_canShowCache.push_back(_filters[index]);
					}
				}

				constexpr auto hasPerk = static_cast<RE::SCRIPT_OUTPUT>(0x11C0);
				std::array<std::byte, sizeof(RE::ConditionCheckParams)> paramStorage;
				std::memcpy(paramStorage.data(), &_params, paramStorage.size());
				auto& params = *std::launder(reinterpret_cast<RE::ConditionCheckParams*>(paramStorage.data()));

				if (((!a_filter && _canShowCache.empty()) ||
						LinearSearch(_canShowCache, a_filter)) &&
					(HasStoredItem(*a_recipe.createdItem) ||
						LinearSearch(_canShowCache, _workshopAlwaysShowIcon) ||
						_isTrueForAllButFunction(a_recipe.conditions, params, hasPerk)) &&
					HasKeyword(a_recipe.benchKeyword)) {
					return true;
				}

				return false;
			}

			map_t<key_type, mapped_type, tbb::scalable_allocator> _lookupCache;
			map_t<const RE::BGSKeyword*, bool, tbb::scalable_allocator> _hasKeywordCache;  // avoid duplicate checks
			std::vector<const RE::BGSKeyword*> _canShowCache = []() {                      // avoid malloc/free overhead
				decltype(_canShowCache) result;
				result.reserve(1u << 4);
				return result;
			}();
			const mapped_type _cobjs = []() {  // pre-filter cobjs
				mapped_type result;
				if (const auto dhandler = RE::TESDataHandler::GetSingleton(); dhandler) {
					for (const auto cobj : dhandler->GetFormArray<RE::BGSConstructibleObject>()) {
						if (cobj &&
							cobj->createdItem &&
							(cobj->createdItem->Is<RE::BGSListForm>() || cobj->createdItem->IsBoundObject())) {
							result.emplace_back(*cobj);
						}
					}
				}
				return result;
			}();
			const std::span<const RE::BGSKeyword*> _filters = []() -> decltype(_filters) {
				if (const auto keywords = RE::BGSKeyword::GetTypedKeywords(); keywords) {
					auto& filters = (*keywords)[stl::to_underlying(RE::BGSKeyword::KeywordType::kRecipeFilter)];
					return { const_cast<const RE::BGSKeyword**>(filters.data()), filters.size() };
				} else {
					return {};
				}
			}();
			const RE::TESForm* const _workshopSplineObject{ REL::Relocation<RE::BGSDefaultObject*>(REL::ID(678816))->form };
			const RE::BGSKeyword* const _badKeyword = []() {
				const auto dobj = RE::BGSDefaultObjectManager::GetSingleton();
				return dobj ?
                           dobj->GetDefaultObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kWorkshopMiscItemKeyword) :
                           nullptr;
			}();
			const RE::BGSKeyword* const _workshopAlwaysShowIcon = []() {
				REL::Relocation<RE::BGSDefaultObject*> dobj{ REL::ID(1581182) };
				const auto form = dobj->form;
				return form ? form->As<RE::BGSKeyword>() : nullptr;
			}();
			const RE::TESObjectSTAT* const _splineEndpointMarker{ *REL::Relocation<RE::TESObjectSTAT**>(REL::ID(1116891)) };
			const RE::NiPointer<RE::TESObjectREFR> _currentWorkshop{ REL::Relocation<RE::ObjectRefHandle*>(REL::ID(737927))->get() };
			const RE::BSAutoWriteLock _extraLock = [&]() {  // avoid constantly locking/unlocking the same extralist
				return RE::BSAutoWriteLock(
					_currentWorkshop && _currentWorkshop->extraList ?
                        &_currentWorkshop->extraList->extraRWLock :
                        nullptr);
			}();
			const RE::BSTSmartPointer<RE::TBO_InstanceData> _currentInstance = [&]() {  // avoid looking up the same instance data every time
				const auto xInst =
					_currentWorkshop && _currentWorkshop->extraList ?
                        _currentWorkshop->extraList->GetByType<RE::ExtraInstanceData>() :
                        nullptr;
				return xInst ? xInst->data : nullptr;
			}();
			const std::vector<std::uint32_t> _storedItems = [&]() {
				std::vector<std::uint32_t> result;
				const auto xWorkshop =
					_currentWorkshop && _currentWorkshop->extraList ?
                        _currentWorkshop->extraList->GetByType<RE::Workshop::ExtraData>() :
                        nullptr;
				if (xWorkshop) {
					for (const auto item : xWorkshop->deletedItems) {
						if (item && item->count > 0) {
							result.push_back(item->formID);
						}
					}
					std::sort(result.begin(), result.end());
				}
				return result;
			}();
			IsTrueForAllButFunction_t* const _isTrueForAllButFunction{ REL::Relocation<IsTrueForAllButFunction_t*>(REL::ID(1182457)).get() };
			const RE::ConditionCheckParams _params = []() {  // use intrinsics to quickly initialize arguments
				static_assert(std::is_trivially_copy_constructible_v<RE::ConditionCheckParams>);
				static_assert(std::is_trivially_destructible_v<RE::ConditionCheckParams>);
				const auto player = RE::PlayerCharacter::GetSingleton();
				RE::ConditionCheckParams params;
				params.actionRef = player;
				params.targetRef = player;
				return params;
			}();
		};

#if 0
		[[nodiscard]] inline bool CompareRecursive(
			const RE::Workshop::WorkshopMenuNode& a_lhs,
			const RE::Workshop::WorkshopMenuNode& a_rhs)
		{
			const auto getChildren = [&](const RE::Workshop::WorkshopMenuNode& a_node) {
				std::vector<const RE::Workshop::WorkshopMenuNode*> children;
				children.reserve(a_node.children.size());
				for (const auto& child : a_node.children) {
					children.push_back(child.get());
				}
				std::sort(children.begin(), children.end(), [](auto&& a_lhs, auto&& a_rhs) noexcept {
#define CASE(a_name)                                                            \
	if (!!a_lhs->a_name != !!a_rhs->a_name || a_lhs->a_name != a_rhs->a_name) { \
		return !!a_lhs->a_name != !!a_rhs->a_name ?                             \
                   !!a_lhs->a_name :                                            \
                   a_lhs->a_name->GetFormID() < a_rhs->a_name->GetFormID();     \
	}
					// clang-format off
					CASE(filterKeyword)
					else CASE(recipe)
					else CASE(form)
					else return false;
					// clang-format on
#undef CONTROL
				});
				return children;
			};

			const auto lChildren = getChildren(a_lhs);
			const auto rChildren = getChildren(a_rhs);

			if (a_lhs.filterKeyword == a_rhs.filterKeyword &&
				a_lhs.children.size() == a_rhs.children.size() &&
				a_lhs.recipe == a_rhs.recipe &&
				a_lhs.sourceFormListRecipe == a_rhs.sourceFormListRecipe &&
				a_lhs.form == a_rhs.form &&
				a_lhs.row == a_rhs.row) {
				for (std::size_t i = 0; i < lChildren.size(); ++i) {
					assert(lChildren[i] && rChildren[i]);
					if (!CompareRecursive(*lChildren[i], *rChildren[i])) {
						return false;
					}
				}
				return true;
			} else {
				const auto [lIt, rIt] = std::mismatch(
					lChildren.begin(),
					lChildren.end(),
					rChildren.begin(),
					rChildren.end(),
					[](auto&& a_lhs, auto&& a_rhs) noexcept {
						return a_lhs->filterKeyword == a_rhs->filterKeyword &&
					           a_lhs->recipe == a_rhs->recipe &&
					           a_lhs->form == a_rhs->form;
					});
				[[maybe_unused]] const auto lPos = lIt - lChildren.begin();
				[[maybe_unused]] const auto rPos = rIt - rChildren.begin();
				return false;
			}
		}
#endif

		inline void FixupChildren(
			const CompareFactory& a_comp,
			RE::Workshop::WorkshopMenuNode& a_node)
		{
			std::stable_sort(a_node.children.begin(), a_node.children.end(), a_comp.less());
			if (const auto last = std::unique(a_node.children.begin(), a_node.children.end(), a_comp.equal_to());
				last != a_node.children.end()) {
				a_node.children.erase(last, a_node.children.end());
			}

			for (std::uint32_t i = 0; i < a_node.children.size(); ++i) {
				const auto& child = a_node.children[i];
				assert(child != nullptr);
				child->column = static_cast<std::uint16_t>(i);
			}
		}

		inline void MakeLeaf(
			NodeFactory a_factory,
			RE::BGSConstructibleObject& a_cobj)
		{
			assert(a_cobj.createdItem != nullptr);
			const auto make = [&](RE::TESForm* a_form) -> decltype(auto) {
				auto& child = a_factory();
				child.recipe = &a_cobj;
				child.form = a_form;
				return child;
			};

			switch (a_cobj.createdItem->GetFormType()) {
			case RE::ENUM_FORM_ID::kFLST:
				for (const auto& flst = static_cast<RE::BGSListForm&>(*a_cobj.createdItem);
					 const auto form : flst.arrayOfForms) {
					make(form).sourceFormListRecipe = &a_cobj;
				}
				break;
			default:
				make(a_cobj.createdItem);
				break;
			}
		}

		inline bool MakeSubMenu(
			LookupTable& a_table,
			const CompareFactory& a_comp,
			NodeFactory a_factory,
			RE::TESForm& a_form)
		{
			assert(!a_form.IsDeleted());

			bool success = false;
			const auto make = [&](RE::BGSKeyword* a_keyword) -> decltype(auto) {
				auto& child = a_factory();
				child.filterKeyword = a_keyword;
				return child;
			};

			switch (a_form.GetFormType()) {
			case RE::ENUM_FORM_ID::kKYWD:
				{
					auto& kywd = static_cast<RE::BGSKeyword&>(a_form);
					const auto& matches = a_table(kywd);
					if (!matches.empty()) {
						auto& child = make(&kywd);
						const auto factory = a_factory.clone(child);
						for (RE::BGSConstructibleObject& cobj : matches) {
							MakeLeaf(factory, cobj);
						}

						success = true;
						FixupChildren(a_comp, child);
					}
				}
				break;
			case RE::ENUM_FORM_ID::kFLST:
				{
					auto& flst = static_cast<RE::BGSListForm&>(a_form);
					const auto front = !flst.arrayOfForms.empty() ? flst.arrayOfForms.front() : nullptr;
					if (front && front->Is<RE::BGSKeyword>()) {
						auto& kywd = static_cast<RE::BGSKeyword&>(*front);
						auto& child = make(&kywd);
						auto& children = child.children;
						const auto factory = a_factory.clone(child);
						for (const auto lform : flst.arrayOfForms) {
							if (lform &&
								!lform->IsDeleted() &&
								MakeSubMenu(a_table, a_comp, factory, *lform) &&
								children.back()->children.empty()) {
								children.pop_back();
							}
						}

						success = true;
						FixupChildren(a_comp, child);
					}
				}
				break;
			default:
				assert(false);
				break;
			}

			return success;
		}

		inline void MakeRootMenu(
			RE::Workshop::WorkshopMenuNode& a_rootNode,
			RE::BGSListForm a_rootList)
		{
			a_rootNode.uniqueID = static_cast<std::uint32_t>(-2);
			a_rootNode.row = static_cast<std::uint16_t>(-2);

			LookupTable table;
			MakeSubMenu(table, {}, { a_rootNode, a_rootNode.uniqueID }, a_rootList);

			if (a_rootNode.children.size() == 1) {
				auto tmp = std::move(a_rootNode.children.front()->children);
				a_rootNode.children = std::move(tmp);
				for (const auto& child : a_rootNode.children) {
					assert(child != nullptr);
					child->parent = &a_rootNode;
				}
			}
			a_rootNode.uniqueID = static_cast<std::uint32_t>(-1);
			a_rootNode.row = 0;
		}

		inline void BuildWorkShopMenuNodeTree()
		{
			const REL::Relocation<RE::ObjectRefHandle*> hCurrentWorkshop{ REL::ID(737927) };
			const REL::Relocation<RE::ObjectRefHandle*> hLastWorkshop{ REL::ID(56095) };
			const REL::Relocation<std::uint16_t*> currentRow{ REL::ID(833923) };
			const REL::Relocation<RE::Workshop::WorkshopMenuNode*> wireNode{ REL::ID(450339) };
			const REL::Relocation<RE::Workshop::WorkshopMenuNode*> rootNode{ REL::ID(1421138) };
			const REL::Relocation<RE::BGSDefaultObject*> rootList{ REL::ID(1514918) };

			std::optional<std::uint32_t> crc;
			if (*hCurrentWorkshop == *hLastWorkshop) {
				std::uint32_t column = 0;
				const auto node = RE::Workshop::GetSelectedWorkshopMenuNode(*currentRow, column);
				std::uint32_t data = 0;
				if (node) {
					data += node->form ? node->form->GetFormID() : 0;
					data += node->filterKeyword ? node->filterKeyword->GetFormID() : 0;
					data += node->row;
				}
				crc = RE::BSCRC32<std::uint32_t>()(data);
			}

			*currentRow = 0;
			rootNode->Clear();

			const auto dhandler = RE::TESDataHandler::GetSingleton();
			const auto currentWorkshop = hCurrentWorkshop->get();
			if (dhandler && currentWorkshop) {
				const auto splineEndpointMarker = *REL::Relocation<RE::TESObjectSTAT**>{ REL::ID(1116891) };
				const auto xInst =
					currentWorkshop->extraList ?
                        currentWorkshop->extraList->GetByType<RE::ExtraInstanceData>() :
                        nullptr;
				const auto inst = xInst ? xInst->data : nullptr;
				for (const auto cobj : dhandler->GetFormArray<RE::BGSConstructibleObject>()) {
					if (cobj &&
						cobj->createdItem &&
						cobj->createdItem == splineEndpointMarker &&
						currentWorkshop->HasKeyword(cobj->benchKeyword, inst.get())) {
						wireNode->recipe = cobj;
						wireNode->form = cobj->createdItem;
						break;
					}
				}
			}

			if (const auto form = rootList->form; form && form->Is<RE::BGSListForm>()) {
				MakeRootMenu(*rootNode, *static_cast<RE::BGSListForm*>(form));
			}

			if (crc) {
				rootNode->FindAndSetSelectedNode(0, *crc, *currentRow);
			}
		}
	}

	inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ REL::ID(1515994) };
		constexpr std::size_t size = 0x24F;
		REL::safe_fill(target.address(), REL::NOP, size);
		stl::asm_jump(target.address(), size, reinterpret_cast<std::uintptr_t>(detail::BuildWorkShopMenuNodeTree));
		logger::info("installed WorkshopMenu patch"sv);
	}
}
