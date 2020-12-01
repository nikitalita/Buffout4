#include "Crash/Introspection/Introspection.h"

#include "Crash/Modules/ModuleHandler.h"

namespace Crash::Introspection::F4
{
	using filter_results = std::vector<std::pair<std::string, std::string>>;

	namespace BSResource
	{
		class LooseFileStreamBase
		{
		public:
			using value_type = RE::BSResource::LooseFileStreamBase;

			static void filter(
				filter_results& a_results,
				const void* a_ptr) noexcept
			{
				const auto stream = static_cast<const value_type*>(a_ptr);

				try {
					const auto dirName = stream->GetDirName();
					a_results.emplace_back(
						"Directory Name"sv,
						dirName);
				} catch (...) {}

				try {
					const auto fileName = stream->GetFileName();
					a_results.emplace_back(
						"File Name"sv,
						fileName);
				} catch (...) {}

				try {
					const auto prefix = stream->GetPrefix();
					a_results.emplace_back(
						"Prefix"sv,
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
					const void* a_ptr) noexcept
				{
					const auto function = static_cast<const value_type*>(a_ptr);

					try {
						const std::string_view name = function->name;
						a_results.emplace_back(
							"Function"sv,
							name);
					} catch (...) {}

					try {
						const std::string_view objName = function->objName;
						a_results.emplace_back(
							"Object"sv,
							objName);
					} catch (...) {}

					try {
						const std::string_view stateName = function->stateName;
						if (!stateName.empty()) {
							a_results.emplace_back(
								"State"sv,
								stateName);
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
				const void* a_ptr) noexcept
			{
				const auto info = static_cast<const value_type*>(a_ptr);

				try {
					const std::string_view name = info->name;
					a_results.emplace_back(
						"Name"sv,
						name);
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
			const void* a_ptr) noexcept
		{
			const auto object = static_cast<const value_type*>(a_ptr);

			try {
				const auto name = object->GetName();
				a_results.emplace_back(
					"Name"sv,
					name);
			} catch (...) {}
		}
	};

	class NiStream
	{
	public:
		using value_type = RE::NiStream;

		static void filter(
			filter_results& a_results,
			const void* a_ptr) noexcept
		{
			const auto stream = static_cast<const value_type*>(a_ptr);

			try {
				const auto fileName = stream->GetFileName();
				a_results.emplace_back(
					"File Name"sv,
					fileName);
			} catch (...) {}
		}
	};

	class NiTexture
	{
	public:
		using value_type = RE::NiTexture;

		static void filter(
			filter_results& a_results,
			const void* a_ptr) noexcept
		{
			const auto texture = static_cast<const value_type*>(a_ptr);

			try {
				const auto name = texture->GetName();
				a_results.emplace_back(
					"Name"sv,
					name);
			} catch (...) {}
		}
	};

	class TESForm
	{
	public:
		using value_type = RE::TESForm;

		static void filter(
			filter_results& a_results,
			const void* a_ptr) noexcept
		{
			const auto form = static_cast<const value_type*>(a_ptr);

			try {
				const auto file = form->GetDescriptionOwnerFile();
				const auto filename = file ? file->GetFilename() : ""sv;
				a_results.emplace_back(
					"File"sv,
					filename);
			} catch (...) {}

			try {
				const auto formFlags = form->GetFormFlags();
				a_results.emplace_back(
					"Flags"sv,
					fmt::format(
						FMT_STRING("0x{:08X}"),
						formFlags));
			} catch (...) {}

			try {
				const auto formID = form->GetFormID();
				a_results.emplace_back(
					"Form ID"sv,
					fmt::format(
						FMT_STRING("0x{:08X}"),
						formID));
			} catch (...) {}
		}
	};

	class TESFullName
	{
	public:
		using value_type = RE::TESFullName;

		static void filter(
			filter_results& a_results,
			const void* a_ptr) noexcept
		{
			const auto component = static_cast<const value_type*>(a_ptr);

			try {
				const auto fullName = component->GetFullName();
				a_results.emplace_back(
					"Full Name"sv,
					fullName);
			} catch (...) {}
		}
	};

	class TESObjectREFR
	{
	public:
		using value_type = RE::TESObjectREFR;

		static void filter(
			filter_results& a_results,
			const void* a_ptr) noexcept
		{
			const auto ref = static_cast<const value_type*>(a_ptr);

			try {
				const auto objRef = ref->data.objectReference;
				if (objRef) {
					filter_results xResults;
					TESForm::filter(xResults, objRef);

					a_results.emplace_back(
						"Object Reference"sv,
						""sv);
					for (auto& [key, value] : xResults) {
						a_results.emplace_back(
							fmt::format(FMT_STRING("\t{}"), key),
							std::move(value));
					}
				} else {
					a_results.emplace_back(
						"Object Reference"sv,
						"None"sv);
				}
			} catch (...) {}
		}
	};
}

namespace Crash::Introspection
{
	[[nodiscard]] const Modules::Module* get_module_for_pointer(
		const void* a_ptr,
		stl::span<const module_pointer> a_modules) noexcept
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
		class IType
		{
		public:
			virtual ~IType() = default;

			[[nodiscard]] std::string name() const { return get_name(); }

		protected:
			[[nodiscard]] virtual std::string get_name() const = 0;
		};

		class Integer :
			public IType
		{
		protected:
			[[nodiscard]] std::string get_name() const override { return "(size_t)"s; }  // NOLINT(readability-convert-member-functions-to-static)
		};

		class Pointer :
			public IType
		{
		public:
			Pointer() noexcept = default;

			Pointer(const void* a_ptr, stl::span<const module_pointer> a_modules) noexcept :
				_module(get_module_for_pointer(a_ptr, a_modules))
			{
				if (_module) {
					_ptr = a_ptr;
				}
			}

		protected:
			[[nodiscard]] std::string get_name() const override
			{
				if (_module) {
					const auto address = reinterpret_cast<std::uintptr_t>(_ptr);
					return fmt::format(
						FMT_STRING("(void* -> {}+{:07X})"),
						_module->name(),
						address - _module->address());
				} else {
					return "(void*)"s;
				}
			}

		private:
			const Modules::Module* _module{ nullptr };
			const void* _ptr{ nullptr };
		};

		class Polymorphic :
			public IType
		{
		public:
			explicit Polymorphic(std::string_view a_mangled) noexcept :
				_mangled{ a_mangled }
			{
				// NOLINTNEXTLINE(readability-simplify-subscript-expr)
				assert(_mangled.size() > 1 && _mangled.data()[_mangled.size()] == '\0');
			}

		protected:
			[[nodiscard]] std::string get_name() const override
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
						FMT_STRING("({}*)"),
						std::string_view{ buf.data(), len });
				} else {
					return "(ERROR)"s;
				}
			}

		private:
			std::string_view _mangled;
		};

		class F4Polymorphic :
			public IType
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

		protected:
			[[nodiscard]] std::string get_name() const override
			{
				auto result = _poly.name();
				F4::filter_results xInfo;

				const auto moduleBase = REL::Module::get().base();
				const auto hierarchy = _col->classDescriptor.get();
				const stl::span bases(
					reinterpret_cast<std::uint32_t*>(hierarchy->baseClassArray.offset() + moduleBase),
					hierarchy->numBaseClasses);
				for (const auto rva : bases) {
					const auto base = reinterpret_cast<RE::RTTI::BaseClassDescriptor*>(rva + moduleBase);
					const auto it = FILTERS.find(base->typeDescriptor->mangled_name());
					if (it != FILTERS.end()) {
						const auto root = stl::adjust_pointer<void>(_ptr, -static_cast<std::ptrdiff_t>(_col->offset));
						const auto target = stl::adjust_pointer<void>(root, static_cast<std::ptrdiff_t>(base->pmd.mDisp));
						it->second(xInfo, target);
					}
				}

				if (!xInfo.empty()) {
					for (const auto& [key, value] : xInfo) {
						result += fmt::format(
							FMT_STRING("\n\t\t{}: {}"),
							key,
							value);
					}
				}

				return result;
			}

		private:
			static constexpr auto FILTERS = frozen::make_map({
				std::make_pair(".?AULooseFileStreamBase@?A0xaf4cad8a@BSResource@@"sv, F4::BSResource::LooseFileStreamBase::filter),
				std::make_pair(".?AVNativeFunctionBase@NF_util@BSScript@@"sv, F4::BSScript::NF_util::NativeFunctionBase::filter),
				std::make_pair(".?AVObjectTypeInfo@BSScript@@"sv, F4::BSScript::ObjectTypeInfo::filter),
				std::make_pair(".?AVNiObjectNET@@"sv, F4::NiObjectNET::filter),
				std::make_pair(".?AVNiStream@@"sv, F4::NiStream::filter),
				std::make_pair(".?AVNiTexture@@"sv, F4::NiTexture::filter),
				std::make_pair(".?AVTESForm@@"sv, F4::TESForm::filter),
				std::make_pair(".?AVTESFullName@@"sv, F4::TESFullName::filter),
				std::make_pair(".?AVTESObjectREFR@@"sv, F4::TESObjectREFR::filter),
			});

			Polymorphic _poly;
			const RE::RTTI::CompleteObjectLocator* _col{ nullptr };
			const void* _ptr{ nullptr };
		};

		class String :
			public IType
		{
		public:
			String(std::string_view a_str) noexcept :
				_str(a_str)
			{}

		protected:
			[[nodiscard]] std::string get_name() const override
			{
				return fmt::format(
					FMT_STRING("(char*) \"{}\""),
					_str);
			}

		private:
			std::string_view _str;
		};

		template <class...>
		struct largest_of;

		template <class Last>
		struct largest_of<Last>
		{
			static constexpr std::size_t value = sizeof(Last);
		};

		template <class First, class Second, class... Rest>
		struct largest_of<First, Second, Rest...>
		{
			static constexpr std::size_t value =
				largest_of<
					std::conditional_t<
						(sizeof(First) >= sizeof(Second)),
						First,
						Second>,
					Rest...>::value;
		};

		template <class... Args>
		inline constexpr std::size_t largest_of_v = largest_of<Args...>::value;

		template <class... Args>
		struct aligned_storage_for
		{
			alignas(Args...) std::byte c[largest_of_v<Args...>]{};
			bool free{ true };
		};

		using analysis_storage = aligned_storage_for<
			Integer,
			Pointer,
			Polymorphic,
			F4Polymorphic,
			String>;

		[[nodiscard]] analysis_storage& get_analysis_storage() noexcept
		{
			thread_local analysis_storage storage;
			return storage;
		}

		struct storage_deleter
		{
			void operator()(IType* a_type)
			{
				auto& storage = get_analysis_storage();
				assert(!storage.free);
				std::destroy_at(a_type);
				storage.free = true;
			}
		};

		using analysis_result = std::unique_ptr<IType, storage_deleter>;

		template <class T, class... Args>
		[[nodiscard]] analysis_result make_result(Args&&... a_args)
		{
			auto& storage = get_analysis_storage();
			assert(storage.free);
			const auto ptr = std::construct_at(
				reinterpret_cast<T*>(storage.c),
				std::forward<Args>(a_args)...);
			storage.free = false;

			static_assert(sizeof(T) <= std::extent_v<decltype(storage.c)>);
			static_assert(std::is_base_of_v<IType, T>);

			return analysis_result{ ptr };
		}

		[[nodiscard]] auto analyze_polymorphic(
			void* a_ptr,
			stl::span<const module_pointer> a_modules) noexcept
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
			stl::span<const module_pointer> a_modules) noexcept
			-> analysis_result
		{
			auto poly = analyze_polymorphic(a_ptr, a_modules);
			if (poly) {
				return *std::move(poly);
			}

			auto str = analyze_string(a_ptr);
			if (str) {
				return *std::move(str);
			}

			return make_result<Pointer>(a_ptr, a_modules);
		}

		[[nodiscard]] auto analyze_integer(
			std::size_t a_value,
			stl::span<const module_pointer> a_modules) noexcept
			-> analysis_result
		{
			try {
				if (a_value != 0) {
					*reinterpret_cast<const volatile std::byte*>(a_value);
					return analyze_pointer(reinterpret_cast<void*>(a_value), a_modules);
				}
			} catch (...) {}

			return make_result<Integer>();
		}
	}

	std::vector<std::string> analyze_data(
		stl::span<const std::size_t> a_data,
		stl::span<const module_pointer> a_modules)
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
				results[pos] = result->name();
			});
		return results;
	}
}
