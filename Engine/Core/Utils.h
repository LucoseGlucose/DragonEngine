#pragma once

#ifdef _DEBUG
#define NAME_D3D_OBJECT(obj) Utils::ThrowIfFailed(obj->SetName(L#obj))
#else
#define NAME_D3D_OBJECT(obj)
#endif

#define GETTER(name, obj) inline auto name() { return obj; };
#define STATIC(decl) static inline decl{}

class Utils
{
public:
	static void ThrowIfFailed(HRESULT result);
	static void ThrowIfFailed(HRESULT result, LPCWSTR message);
	static void CrashWithMessage(LPCWSTR message);

	static std::filesystem::path GetPathFromExe(std::filesystem::path path);
	static std::filesystem::path GetPathFromProject(std::filesystem::path path);
	static std::filesystem::path GetPathFromSolution(std::filesystem::path path);

	template<typename T>
	static void RemoveFromVector(std::vector<T>* vec, const T& item)
	{
		vec->erase(std::find(vec->begin(), vec->end(), item));
	}
};