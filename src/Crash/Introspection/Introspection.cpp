#include "Crash/Introspection/Introspection.h"

#include "Crash/Modules/ModuleHandler.h"
#include "Crash/PDB/PdbHandler.h"
#define MAGIC_ENUM_RANGE_MAX 256
#include <magic_enum.hpp>

namespace Crash::Introspection::F4
{
	using filter_results = std::vector<std::pair<std::string, std::string>>;

	[[nodiscard]] std::string quoted(std::string_view a_str)
	{
		return fmt::format("\"{}\""sv, a_str);
	}

	namespace BSResource
	{
		class LooseFileStreamBase
		{
		public:
			using value_type = RE::BSResource::LooseFileStreamBase;

			static void filter(
				filter_results& a_results,
				const void* a_ptr, int tab_depth = 0) noexcept
			{
				const auto stream = static_cast<const value_type*>(a_ptr);

				try {
					const auto dirName = stream->GetDirName();
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Directory Name"sv,
							"",
							tab_depth),
						dirName);
				} catch (...) {}

				try {
					const auto fileName = stream->GetFileName();
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}File Name"sv,
							"",
							tab_depth),
						fileName);
				} catch (...) {}

				try {
					const auto prefix = stream->GetPrefix();
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Prefix"sv,
							"",
							tab_depth),
						prefix);
				} catch (...) {}
			}
		};
	}

	namespace BSScript
	{
		namespace NF_util
		{
			class NativeFunctionBase
			{
			public:
				using value_type = RE::BSScript::NF_util::NativeFunctionBase;

				static void filter(
					filter_results& a_results,
					const void* a_ptr, int tab_depth = 0) noexcept
				{
					const auto function = static_cast<const value_type*>(a_ptr);

					try {
						const std::string_view name = function->name;
						a_results.emplace_back(
							fmt::format(
								"{:\t>{}}Function"sv,
								"",
								tab_depth),
							quoted(name));
					} catch (...) {}

					try {
						const std::string_view objName = function->objName;
						a_results.emplace_back(
							fmt::format(
								"{:\t>{}}Object"sv,
								"",
								tab_depth),
							quoted(objName));
					} catch (...) {}

					try {
						const std::string_view stateName = function->stateName;
						if (!stateName.empty()) {
							a_results.emplace_back(
								fmt::format(
									"{:\t>{}}State"sv,
									"",
									tab_depth),
								quoted(stateName));
						}
					} catch (...) {}
				}
			};
		}

		class ObjectTypeInfo
		{
		public:
			using value_type = RE::BSScript::ObjectTypeInfo;

			static void filter(
				filter_results& a_results,
				const void* a_ptr, int tab_depth = 0) noexcept
			{
				const auto info = static_cast<const value_type*>(a_ptr);

				try {
					const std::string_view name = info->name;
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Name"sv,
							"",
							tab_depth),
						quoted(name));
				} catch (...) {}
			}
		};
	}

	class NiObjectNET
	{
	public:
		using value_type = RE::NiObjectNET;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto object = static_cast<const value_type*>(a_ptr);

			try {
				const auto name = object->GetName();
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Name"sv,
						"",
						tab_depth),
					quoted(name));
			} catch (...) {}
		}
	};

	class NiStream
	{
	public:
		using value_type = RE::NiStream;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto stream = static_cast<const value_type*>(a_ptr);

			try {
				const auto fileName = stream->GetFileName();
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}File Name"sv,
						""sv,
						tab_depth),
					quoted(fileName));
			} catch (...) {}
			try {
				const auto& header = stream->bsStreamHeader;
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Header"sv,
						"",
						tab_depth),
					fmt::format(
						"author: {} version: {} processScript: {} exportScript: {}",
						header.author,
						header.version,
						header.processScript,
						header.exportScript));
			} catch (...) {}

			try {
				const auto lastLoadedRTTI = stream->lastLoadedRTTI;
				if (lastLoadedRTTI && lastLoadedRTTI[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}lastLoadedRTTI"sv,
							"",
							tab_depth),
						quoted(lastLoadedRTTI));
			} catch (...) {}

			try {
				const auto inputFilePath = stream->fileName;
				if (inputFilePath && inputFilePath[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}fileName"sv,
							"",
							tab_depth),
						quoted(inputFilePath));
			} catch (...) {}

			try {
				const auto filePath = stream->filePath;
				if (filePath && filePath[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}filePath"sv,
							"",
							tab_depth),
						quoted(filePath));
			} catch (...) {}
		}
	};

	class NiTexture
	{
	public:
		using value_type = RE::NiTexture;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto texture = static_cast<const value_type*>(a_ptr);
			if (!texture)
				return;
			try {
				const auto name = texture ? texture->name.c_str() : ""sv;
				if (!name.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Name"sv,
							"",
							tab_depth),
						quoted(name));
			} catch (...) {}

			try {
				const auto name = texture->GetRTTI() ? texture->GetRTTI()->GetName() : ""sv;
				if (!name.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}RTTIName"sv,
							"" /*,
							tab_depth*/
							),
						quoted(name));
			} catch (...) {}
		}
	};

	template <class T = RE::TESForm>
	class TESForm
	{
	public:
		using value_type = T;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto form = static_cast<const value_type*>(a_ptr);

			try {
				const auto file = form->GetDescriptionOwnerFile();
				const auto filename = file ? file->GetFilename() : ""sv;
				if (!filename.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}File"sv,
							""sv,
							tab_depth),
						quoted(filename));
			} catch (...) {}

			try {
				const auto formFlags = form->GetFormFlags();
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Flags"sv,
						""sv,
						tab_depth),
					fmt::format(
						"0x{:08X}"sv,
						formFlags));
			} catch (...) {}

			try {
				const auto editorID = form->GetFormEditorID();
				if (editorID && editorID[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}EditorID"sv,
							""sv,
							tab_depth),
						quoted(editorID));
			} catch (...) {}

			try {
				const auto formID = form->GetFormID();
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}FormID"sv,
						""sv,
						tab_depth),
					fmt::format(
						"0x{:08X}"sv,
						formID));
			} catch (...) {}

			try {
				const auto formType = form->GetFormType();
				const auto formTypeName = magic_enum::enum_name(formType);
				if (!formTypeName.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}FormType"sv,
							"",
							tab_depth),
						fmt::format(
							"{} ({:02})"sv,
							formTypeName, stl::to_underlying(formType)));
			} catch (...) {}
		}
	};

	class TESFullName
	{
	public:
		using value_type = RE::TESFullName;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto component = static_cast<const value_type*>(a_ptr);

			if (!component)
				return;
			try {
				const auto fullName = component->GetFullName();
				if (fullName && fullName[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}GetFullName"sv,
							""sv,
							tab_depth),
						quoted(fullName));
			} catch (...) {}
		}
	};

	class TESObjectREFR
	{
	public:
		using value_type = RE::TESObjectREFR;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto ref = static_cast<const value_type*>(a_ptr);

			try {
				const auto objRef = ref->data.objectReference;
				if (objRef) {
					filter_results xResults;
					TESForm<RE::TESForm>::filter(xResults, objRef);

					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Object Reference"sv,
							tab_depth),
						""sv);
					for (auto& [key, value] : xResults) {
						a_results.emplace_back(
							fmt::format("\t{}"sv, key),
							std::move(value));
					}
				} else {
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Object Reference"sv,
							tab_depth),
						"None"sv);
				}
			} catch (...) {}
		}
	};

	class BSShaderProperty
	{
	public:
		using value_type = RE::BSShaderProperty;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto form = static_cast<const value_type*>(a_ptr);

			try {
				const auto formFlags = form->flags;
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Flags"sv,
						""sv,
						tab_depth),
					fmt::format(
						"0x{:08X}"sv,
						formFlags));
			} catch (...) {}
			try {
				const auto name = form->name.c_str();
				if (name && name[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Name"sv,
							""sv,
							tab_depth),
						quoted(name));
			} catch (...) {}
			try {
				const auto rttiname = form->GetRTTI() ? form->GetRTTI()->GetName() : ""sv;
				if (!rttiname.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}RTTIName"sv,
							""sv,
							tab_depth),
						quoted(rttiname));
			} catch (...) {}
		}
	};
	class NiAVObject
	{
	public:
		using value_type = RE::NiAVObject;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto object = static_cast<const value_type*>(a_ptr);
			if (!object)
				return;
			try {
				const auto name = object ? object->name.c_str() : ""sv;
				if (!name.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Name"sv,
							""sv,
							tab_depth),
						quoted(name));
			} catch (...) {}

			try {
				const auto name = object->GetRTTI() ? object->GetRTTI()->GetName() : ""sv;
				if (!name.empty())
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}RTTIName"sv,
							""sv,
							tab_depth),
						quoted(name));
			} catch (...) {}

			try {
				const auto flags = object->GetFlags();
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Flags"sv,
						""sv,
						tab_depth),
					fmt::format(
						"{}"sv,
						flags));
			} catch (...) {}

			try {
				const auto parent = object->parent;
				if (parent) {
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Checking Parent"sv,
							""sv,
							tab_depth),
						fmt::format(
							""sv));
					filter(a_results, parent, tab_depth + 1);
				}
			} catch (...) {}
		}
	};

	class TESQuest
	{
	public:
		using value_type = RE::TESQuest;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto object = static_cast<const value_type*>(a_ptr);
			if (!object)
				return;
			try {
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Already Run Quest"sv,
						"",
						tab_depth),
					fmt::format(
						"{}",
						object->alreadyRun));
				a_results.emplace_back(
					fmt::format(
						"{:\t>{}}Current Stage"sv,
						"",
						tab_depth),
					fmt::format(
						"{}",
						object->currentStage));
			} catch (...) {}
		};
	};

	class ExtraTextDisplayData
	{
	public:
		using value_type = RE::ExtraTextDisplayData;

		static void filter(
			filter_results& a_results,
			const void* a_ptr, int tab_depth = 0) noexcept
		{
			const auto object = static_cast<const value_type*>(a_ptr);

			try {
				const auto name = object->displayName.c_str();
				if (name && name[0])
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Display Name"sv,
							"",
							tab_depth),
						quoted(name));
			} catch (...) {}
			try {
				const auto& displayNameText = object->displayNameText;
				if (displayNameText)
					TESForm<RE::BGSMessage>::filter(a_results, displayNameText, tab_depth + 1);
			} catch (...) {}
			try {
				const auto quest = object->ownerQuest;
				if (quest) {
					a_results.emplace_back(
						fmt::format(
							"{:\t>{}}Owner Quest"sv,
							""sv,
							tab_depth),
						""sv);
					TESQuest::filter(a_results, quest, tab_depth + 1);
				}
			} catch (...) {}
		};
	};

}

namespace Crash::Introspection
{
	[[nodiscard]] const Modules::Module* get_module_for_pointer(
		const void* a_ptr,
		std::span<const module_pointer> a_modules) noexcept
	{
		const auto it = std::lower_bound(
			a_modules.rbegin(),
			a_modules.rend(),
			reinterpret_cast<std::uintptr_t>(a_ptr),
			[](auto&& a_lhs, auto&& a_rhs) noexcept {
				return a_lhs->address() >= a_rhs;
			});
		return it != a_modules.rend() && (*it)->in_range(a_ptr) ? it->get() : nullptr;
	}

	namespace detail
	{
		class Integer
		{
		public:
			Integer(std::size_t a_value) noexcept :
				_value(a_value),
				name_string(a_value >> 63 ?
                                fmt::format("(size_t) [uint: {} int: {}]"s, _value, static_cast<std::make_signed_t<size_t>>(_value)) :
                                fmt::format("(size_t) [{}]"s, _value))
			{
			}

			[[nodiscard]] std::string name() const { return name_string; }

		private:
			const std::size_t _value;
			const std::string name_string;
		};

		class Pointer
		{
		public:
			Pointer() noexcept = default;

			Pointer(const void* a_ptr, std::span<const module_pointer> a_modules) noexcept :
				_module(get_module_for_pointer(a_ptr, a_modules))
			{
				if (_module) {
					_ptr = a_ptr;
				}
			}

			[[nodiscard]] std::string name() const
			{
				if (_module) {
					const auto address = reinterpret_cast<std::uintptr_t>(_ptr);
					const auto pdbDetails = Crash::PDB::pdb_details(_module->path(), address - _module->address());
					const auto assembly = _module->assembly((const void*)address);
					if (!pdbDetails.empty())
						return fmt::format(
							"(void* -> {}+{:07X}\t{} | {})"sv,
							_module->name(),
							address - _module->address(),
							assembly,
							pdbDetails);
					return fmt::format(
						"(void* -> {}+{:07X}\t{})"sv,
						_module->name(),
						address - _module->address(),
						assembly);
				} else {
					return "(void*)"s;
				}
			}

		private:
			const Modules::Module* _module{ nullptr };
			const void* _ptr{ nullptr };
		};

		class Polymorphic
		{
		public:
			explicit Polymorphic(std::string_view a_mangled) noexcept :
				_mangled{ a_mangled }
			{
				// NOLINTNEXTLINE(readability-simplify-subscript-expr)
				assert(_mangled.size() > 1 && _mangled.data()[_mangled.size()] == '\0');
			}

			[[nodiscard]] std::string name() const
			{
				const auto demangle = [](const char* a_in, char* a_out, std::uint32_t a_size) {
					static std::mutex m;
					std::lock_guard l{ m };
					return WinAPI::UnDecorateSymbolName(
						a_in,
						a_out,
						a_size,
						(WinAPI::UNDNAME_NO_MS_KEYWORDS) |
							(WinAPI::UNDNAME_NO_FUNCTION_RETURNS) |
							(WinAPI::UNDNAME_NO_ALLOCATION_MODEL) |
							(WinAPI::UNDNAME_NO_ALLOCATION_LANGUAGE) |
							(WinAPI::UNDNAME_NO_THISTYPE) |
							(WinAPI::UNDNAME_NO_ACCESS_SPECIFIERS) |
							(WinAPI::UNDNAME_NO_THROW_SIGNATURES) |
							(WinAPI::UNDNAME_NO_RETURN_UDT_MODEL) |
							(WinAPI::UNDNAME_NAME_ONLY) |
							(WinAPI::UNDNAME_NO_ARGUMENTS) |
							static_cast<std::uint32_t>(0x8000));  // Disable enum/class/struct/union prefix
				};

				std::array<char, 0x1000> buf{ '\0' };
				const auto len = demangle(
					_mangled.data() + 1,
					buf.data(),
					static_cast<std::uint32_t>(buf.size()));

				if (len != 0) {
					return fmt::format(
						"({}*)"sv,
						std::string_view{ buf.data(), len });
				} else {
					return "(ERROR)"s;
				}
			}

		private:
			std::string_view _mangled;
		};

		class F4Polymorphic
		{
		public:
			F4Polymorphic(
				std::string_view a_mangled,
				const RE::RTTI::CompleteObjectLocator* a_col,
				const void* a_ptr) noexcept :
				_poly{ a_mangled },
				_col{ a_col },
				_ptr{ a_ptr }
			{
				assert(_col != nullptr);
				assert(_ptr != nullptr);
			}

			[[nodiscard]] std::string name() const
			{
				auto result = _poly.name();
				F4::filter_results xInfo;

				const auto moduleBase = REL::Module::get().base();
				const auto hierarchy = _col->classDescriptor.get();
				const std::span bases(
					reinterpret_cast<std::uint32_t*>(hierarchy->baseClassArray.offset() + moduleBase),
					hierarchy->numBaseClasses);
				for (const auto rva : bases) {
					const auto base = reinterpret_cast<RE::RTTI::BaseClassDescriptor*>(rva + moduleBase);
					const auto it = FILTERS.find(base->typeDescriptor->mangled_name());
					if (it != FILTERS.end()) {
						const auto root = stl::adjust_pointer<void>(_ptr, -static_cast<std::ptrdiff_t>(_col->offset));
						const auto target = stl::adjust_pointer<void>(root, static_cast<std::ptrdiff_t>(base->pmd.mDisp));
						it->second(xInfo, target, 0);
					}
					logger::info("Found unhandled type:\t{}\t{}"sv, result, base->typeDescriptor->mangled_name());
				}

				if (!xInfo.empty()) {
					for (const auto& [key, value] : xInfo) {
						result += fmt::format(
							"\n\t\t{}: {}"sv,
							key,
							value);
					}
				}

				return result;
			}

		private:
			static constexpr auto FILTERS = frozen::make_map({
				std::make_pair(".?AULooseFileStreamBase@?A0xaf4cad8a@BSResource@@"sv, F4::BSResource::LooseFileStreamBase::filter),
				std::make_pair(".?AVBSShaderProperty@@"sv, F4::BSShaderProperty::filter),
				std::make_pair(".?AVCharacter@@"sv, F4::TESForm<RE::PlayerCharacter>::filter),
				std::make_pair(".?AVExtraTextDisplayData@@"sv, F4::ExtraTextDisplayData::filter),
				std::make_pair(".?AVNativeFunctionBase@NF_util@BSScript@@"sv, F4::BSScript::NF_util::NativeFunctionBase::filter),
				std::make_pair(".?AVNiAVObject@@"sv, F4::NiAVObject::filter),
				std::make_pair(".?AVNiObjectNET@@"sv, F4::NiObjectNET::filter),
				std::make_pair(".?AVNiStream@@"sv, F4::NiStream::filter),
				std::make_pair(".?AVNiTexture@@"sv, F4::NiTexture::filter),
				std::make_pair(".?AVObjectTypeInfo@BSScript@@"sv, F4::BSScript::ObjectTypeInfo::filter),
				std::make_pair(".?AVPlayerCharacter@@"sv, F4::TESForm<RE::PlayerCharacter>::filter),
				std::make_pair(".?AVTESFaction@@"sv, F4::TESForm<RE::TESFaction>::filter),
				std::make_pair(".?AVTESForm@@"sv, F4::TESForm<RE::TESForm>::filter),
				std::make_pair(".?AVTESFullName@@"sv, F4::TESFullName::filter),
				std::make_pair(".?AVTESNPC@@"sv, F4::TESForm<RE::TESNPC>::filter),
				std::make_pair(".?AVTESObjectCELL@@"sv, F4::TESForm<RE::TESObjectCELL>::filter),
				std::make_pair(".?AVTESObjectREFR@@"sv, F4::TESObjectREFR::filter),
				std::make_pair(".?AVTESQuest@@"sv, F4::TESQuest::filter),
			});

			Polymorphic _poly;
			const RE::RTTI::CompleteObjectLocator* _col{ nullptr };
			const void* _ptr{ nullptr };
		};

		class String
		{
		public:
			String(std::string_view a_str) noexcept :
				_str(a_str)
			{}

			[[nodiscard]] std::string name() const
			{
				return fmt::format("(char*) \"{}\""sv, _str);
			}

		private:
			std::string_view _str;
		};

		using analysis_result = std::variant<
			Integer,
			Pointer,
			Polymorphic,
			F4Polymorphic,
			String>;

		template <class T, class... Args>
		[[nodiscard]] analysis_result make_result(Args&&... a_args) noexcept(
			std::is_nothrow_constructible_v<T, Args...>)
		{
			return analysis_result(std::in_place_type_t<T>{}, std::forward<Args>(a_args)...);
		}

		[[nodiscard]] auto analyze_polymorphic(
			void* a_ptr,
			std::span<const module_pointer> a_modules) noexcept
			-> std::optional<analysis_result>
		{
			try {
				const auto vtable = *reinterpret_cast<void**>(a_ptr);
				const auto mod = get_module_for_pointer(vtable, a_modules);
				if (!mod || !mod->in_rdata_range(vtable)) {
					return std::nullopt;
				}

				const auto col =
					*reinterpret_cast<RE::RTTI::CompleteObjectLocator**>(
						reinterpret_cast<std::size_t*>(vtable) - 1);
				if (mod != get_module_for_pointer(col, a_modules) || !mod->in_rdata_range(col)) {
					return std::nullopt;
				}

				const auto typeDesc =
					reinterpret_cast<RE::RTTI::TypeDescriptor*>(
						mod->address() + col->typeDescriptor.offset());
				if (mod != get_module_for_pointer(typeDesc, a_modules) || !mod->in_data_range(typeDesc)) {
					return std::nullopt;
				}

				if (*reinterpret_cast<const void**>(typeDesc) != mod->type_info()) {
					return std::nullopt;
				}

				if (_stricmp(mod->name().data(), util::module_name().c_str()) == 0) {
					return make_result<F4Polymorphic>(typeDesc->mangled_name(), col, a_ptr);
				} else {
					return make_result<Polymorphic>(typeDesc->mangled_name());
				}
			} catch (...) {
				return std::nullopt;
			}
		}

		[[nodiscard]] auto analyze_string(void* a_ptr) noexcept
			-> std::optional<analysis_result>
		{
			try {
				const auto printable = [](char a_ch) noexcept {
					if (' ' <= a_ch && a_ch <= '~') {
						return true;
					} else {
						switch (a_ch) {
						case '\t':
						case '\n':
							return true;
						default:
							return false;
						}
					}
				};

				const auto str = static_cast<const char*>(a_ptr);
				constexpr std::size_t max = 1000;
				std::size_t len = 0;
				for (; len < max && str[len] != '\0'; ++len) {
					if (!printable(str[len])) {
						return std::nullopt;
					}
				}

				if (len == 0 || len >= max) {
					return std::nullopt;
				}

				return make_result<String>(std::string_view{ str, len });
			} catch (...) {
				return std::nullopt;
			}
		}

		[[nodiscard]] auto analyze_pointer(
			void* a_ptr,
			std::span<const module_pointer> a_modules) noexcept
			-> analysis_result
		{
			if (auto poly = analyze_polymorphic(a_ptr, a_modules); poly) {
				return *std::move(poly);
			}

			if (auto str = analyze_string(a_ptr); str) {
				return *std::move(str);
			}

			return make_result<Pointer>(a_ptr, a_modules);
		}

		[[nodiscard]] auto analyze_integer(
			std::size_t a_value,
			std::span<const module_pointer> a_modules) noexcept
			-> analysis_result
		{
			try {
				if (a_value != 0) {
					*reinterpret_cast<const volatile std::byte*>(a_value);
					return analyze_pointer(reinterpret_cast<void*>(a_value), a_modules);
				}
			} catch (...) {}

			return make_result<Integer>(a_value);
		}
	}

	std::vector<std::string> analyze_data(
		std::span<const std::size_t> a_data,
		std::span<const module_pointer> a_modules)
	{
		std::vector<std::string> results;
		results.resize(a_data.size());
		std::for_each(
			std::execution::par_unseq,
			a_data.begin(),
			a_data.end(),
			[&](auto& a_val) {
				const auto result = detail::analyze_integer(a_val, a_modules);
				const auto pos = std::addressof(a_val) - a_data.data();
				results[pos] = std::visit(
					[](const auto& a_analysis) { return a_analysis.name(); },
					result);
			});
		return results;
	}
}
