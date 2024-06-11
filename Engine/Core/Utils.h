#pragma once

#ifdef _DEBUG
#define NAME_D3D_OBJECT(obj) Utils::ThrowIfFailed(obj->SetName(L#obj))
#else
#define NAME_D3D_OBJECT(obj)
#endif

#define GETTER(name, obj) inline auto name() { return obj; };
#define SETTER(name, obj) inline void name(auto val) { obj = val; };

#define STATIC(decl) static inline decl{}
#define INLINE(decl) inline decl{}

namespace Utils
{
	void ThrowIfFailed(HRESULT result);
	void ThrowIfFailed(HRESULT result, LPCWSTR message);
	void CrashWithMessage(LPCWSTR message);

	std::filesystem::path GetPathFromExe(std::filesystem::path path);
	std::filesystem::path GetPathFromProject(std::filesystem::path path);
	std::filesystem::path GetPathFromSolution(std::filesystem::path path);

	template<typename T>
	void RemoveFromVector(std::vector<T>* vec, const T& item)
	{
		vec->erase(std::find(vec->begin(), vec->end(), item));
	}
};