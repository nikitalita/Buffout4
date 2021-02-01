#pragma once

namespace Patches::WorkshopMenuPatch
{
	namespace detail
	{
#if 0
		[[nodiscard]] inline bool CompareRecursive(
			const RE::Workshop::WorkshopMenuNode& a_lhs,
			const RE::Workshop::WorkshopMenuNode& a_rhs)
		{
			if (a_lhs.filterKeyword == a_rhs.filterKeyword &&
				a_lhs.children.size() == a_rhs.children.size() &&
				a_lhs.recipe == a_rhs.recipe &&
				a_lhs.sourceFormListRecipe == a_rhs.sourceFormListRecipe &&
				a_lhs.form == a_rhs.form &&
				a_lhs.row == a_rhs.row) {
				const auto getChildren = [](const RE::Workshop::WorkshopMenuNode& a_node) {
					std::vector<const RE::Workshop::WorkshopMenuNode*> children(
						a_node.children.begin(),
						a_node.children.end());
					std::sort(children.begin(), children.end(), LessThan);
					return children;
				};

				const auto lChildren = getChildren(a_lhs);
				const auto rChildren = getChildren(a_rhs);

				for (std::size_t i = 0; i < lChildren.size(); ++i) {
					assert(lChildren[i] && rChildren[i]);
					if (!CompareRecursive(*lChildren[i], *rChildren[i])) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
#endif

		class CompareFactory
		{
		private:
			struct eq_t
			{
				[[nodiscard]] bool operator()(
					const RE::msvc::unique_ptr<RE::Workshop::WorkshopMenuNode>& a_lhs,
					const RE::msvc::unique_ptr<RE::Workshop::WorkshopMenuNode>& a_rhs) const noexcept
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
				[[nodiscard]] bool operator()(
					const RE::msvc::unique_ptr<RE::Workshop::WorkshopMenuNode>& a_lhs,
					const RE::msvc::unique_ptr<RE::Workshop::WorkshopMenuNode>& a_rhs) const noexcept
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
				const auto& child = _parent.children.emplace_back(new RE::Workshop::WorkshopMenuNode);
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
			RE::Workshop::WorkshopMenuNode& _parent;
			std::uint32_t& _uniqueID;
		};

		class LookupTable
		{
		public:
			using key_type = RE::BGSKeyword*;
			using mapped_type = std::vector<std::reference_wrapper<RE::BGSConstructibleObject>>;

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
			using WorkshopCanShowRecipe_t = bool(RE::BGSConstructibleObject* a_recipe, RE::BGSKeyword* a_filter);

			[[nodiscard]] bool WorkshopCanShowRecipe(RE::BGSConstructibleObject& a_recipe, RE::BGSKeyword* a_filter)
			{
				assert(_currentWorkshop != nullptr);
				assert(a_recipe.createdItem != nullptr);
				assert(a_recipe.createdItem->IsObject() || a_recipe.createdItem->Is<RE::BGSListForm>());

				_canShowCache.clear();
				_canShowCache.reserve(a_recipe.filterKeywords.size);
				for (std::uint32_t i = 0; i < a_recipe.filterKeywords.size; ++i) {
					const auto index = a_recipe.filterKeywords.array[i].keywordIndex;
					if (index < _filters.size()) {
						_canShowCache.push_back(_filters[index]);
					}
				}

				std::sort(_canShowCache.begin(), _canShowCache.end());
				const auto hasKeyword = [&](const RE::BGSKeyword* a_keyword) noexcept {
					const auto it = std::lower_bound(_canShowCache.begin(), _canShowCache.end(), a_keyword);
					return it != _canShowCache.end() && *it == a_keyword;
				};

				constexpr auto hasPerk = static_cast<RE::SCRIPT_OUTPUT>(0x11C0);
				RE::ConditionCheckParams params;
				params.actionRef = _player.get();
				params.targetRef = _player.get();
				if (((!a_filter && _canShowCache.empty()) || hasKeyword(a_filter)) &&
					(hasKeyword(_workshopAlwaysShowIcon) || a_recipe.conditions.IsTrueForAllButFunction(params, hasPerk)) &&
					_currentWorkshop->HasKeyword(a_recipe.benchKeyword, _currentInstance.get())) {
					return true;
				}

				return false;
			}

			std::map<key_type, mapped_type> _lookupCache;
			std::vector<const RE::BGSKeyword*> _canShowCache;
			const mapped_type _cobjs = []() {
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
			const RE::BGSKeyword* const _badKeyword = []() {
				const auto dobj = RE::BGSDefaultObjectManager::GetSingleton();
				return dobj ?
                           dobj->GetDefaultObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kWorkshopMiscItemKeyword) :
                           nullptr;
			}();
			const RE::BGSKeyword* const _workshopAlwaysShowIcon = []() {
				REL::Relocation<RE::BGSDefaultObject*> workshopAlwaysShowIcon{ REL::ID(1581182) };
				const auto form = workshopAlwaysShowIcon->form;
				return form ? form->As<RE::BGSKeyword>() : nullptr;
			}();
			const RE::NiPointer<RE::TESObjectREFR> _currentWorkshop{ REL::Relocation<RE::ObjectRefHandle*>(REL::ID(737927))->get() };
			const RE::BSTSmartPointer<RE::TBO_InstanceData> _currentInstance = [&]() {
				const auto xInst =
					_currentWorkshop && _currentWorkshop->extraList ?
                        _currentWorkshop->extraList->GetByType<RE::ExtraInstanceData>() :
                        nullptr;
				return xInst ? xInst->data : nullptr;
			}();
			const RE::NiPointer<RE::PlayerCharacter> _player{ RE::PlayerCharacter::GetSingleton() };
		};

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

		inline void MakeLeaf(NodeFactory a_factory, RE::BGSConstructibleObject& a_cobj)
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
					const auto front = flst.arrayOfForms.size() >= 2 ?  // must have at least 1 keyword + 1 element to show up
                                           flst.arrayOfForms.front() :
                                           nullptr;
					if (front && front->Is<RE::BGSKeyword>()) {
						auto& kywd = static_cast<RE::BGSKeyword&>(*front);
						auto& child = make(&kywd);
						auto& children = child.children;
						const auto factory = a_factory.clone(child);
						for (std::uint32_t i = 1; i < flst.arrayOfForms.size(); ++i) {
							if (const auto lform = flst.arrayOfForms[i]; lform && !lform->IsDeleted()) {
								if (MakeSubMenu(a_table, a_comp, factory, *lform) && children.back()->children.empty()) {
									children.pop_back();
								}
							}
						}

						success = true;
						FixupChildren(a_comp, child);
					}
				}
				break;
			default:
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

			const CompareFactory comp;
			LookupTable table;
			MakeSubMenu(table, comp, { a_rootNode, a_rootNode.uniqueID }, a_rootList);

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
						rootNode->recipe = cobj;
						rootNode->form = cobj->createdItem;
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

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ REL::ID(1515994) };
		constexpr std::size_t size = 0x24F;
		REL::safe_fill(target.address(), REL::NOP, size);
		stl::asm_jump(target.address(), size, reinterpret_cast<std::uintptr_t>(detail::BuildWorkShopMenuNodeTree));
		logger::info("installed WorkshopMenuPatch"sv);
	}
}
