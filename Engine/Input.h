#pragma once



class Input
{
	static inline std::map<int, bool> keyMap{};
	static inline std::map<int, bool> prevKeyMap{};

	static inline std::map<int, bool> mouseMap{};
	static inline std::map<int, bool> prevMouseMap{};

	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static inline bool scrolled{};
	static inline float scrollValue{};

	static inline XMFLOAT2 lastMousePos{};
	static inline XMFLOAT2 mousePos{};

public:
	static void Init();
	static void Update();

	static bool GetKey(int key);
	static bool GetKeyDown(int key);
	static bool GetKeyUp(int key);

	static XMFLOAT2 GetMousePosition();

	static bool GetMouseButton(int button);
	static bool GetMouseButtonDown(int button);
	static bool GetMouseButtonUp(int button);

	static float GetMouseScrollDelta();
	static XMFLOAT2 GetMousePosDelta();
};