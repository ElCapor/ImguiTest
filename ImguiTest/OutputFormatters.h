#pragma once

#include "Profiler.h"

#if USE_PROFILER


#define JSON_OUTPUT  1		// Set to 1/0 to enable/disable json output formatting
#define IMGUI_OUTPUT 0		// Set to 1/0 to enable/disable imgui output formatting


#if JSON_OUTPUT
#include <json.hpp>	// Json.hpp from Nlohman json library is assumed to be part of additional include directories (otherwise, JSON_OUTPUT can be turned off)
using json = nlohmann::json;
#endif


namespace Profiler
{
	namespace Formatters
	{

#if JSON_OUTPUT

		class JsonFormatter
		{
		public:
			void dump_to_json(const char* filePath);
		private:
			void dump_node(json& nodeJson, ProfilingMgr::node* nodeToDump);
		};

// Dumps to a json file the information of one frame
#define DUMP_TO_JSON(filePath) Profiler::Formatters::JsonFormatter dumper; dumper.dump_to_json(filePath);

#else
	#define DUMP_TO_JSON(filePath)
#endif	// JSON_OUTPUT



#if IMGUI_OUTPUT
		class ImGuiFormatter
		{
		public:
			void on_gui();	// Assumes ImGui library is initialized, and this is being called as part of the ImGui application code
		private:
			void dump_node(ProfilingMgr::node* nodeToDump);

			//int m_valOffset = 0;
		};

#define DUMP_TO_IMGUI() Profiler::Formatters::ImGuiFormatter dumper; dumper.on_gui();

#else
#define DUMP_TO_IMGUI()
#endif	// IMGUI_OUTPUT

	}
}

#else
	#define DUMP_TO_JSON(filePath)
	#define DUMP_TO_IMGUI()
#endif	// USE_PROFILER