#include "OutputFormatters.h"


#if USE_PROFILER

#if JSON_OUTPUT
#include <fstream>
#include <iomanip>
#endif

#if IMGUI_OUTPUT
#include <imgui.h>	// Imgui.h is assumed to be part of additional include directories (otherwise, IMGUI_OUTPUT can be turned off)
#include <algorithm>
#endif


namespace Profiler
{
	namespace Formatters
	{
#if JSON_OUTPUT
		void JsonFormatter::dump_to_json(const char* filePath)
		{
			std::ofstream file(filePath);
			if (file.is_open())
			{
				ProfilingMgr::node* root = ProfilingMgr::get_instance().get_root();

				json rootStats;
				dump_node(rootStats, root->m_child);

				file << std::setw(4) << rootStats;// << randomStats;
			}
		}

		void JsonFormatter::dump_node(json& nodeJson, ProfilingMgr::node* nodeToDump)
		{
			if (nodeToDump == nullptr)
				return;

			// Save the stats of the current node in "stats"
			nodeJson["1) ID"] = nodeToDump->m_id;
			nodeJson["2) Call count"] = nodeToDump->m_stats.m_callCount;
			nodeJson["3) Total cycles"] = nodeToDump->m_stats.m_totalCycles;

			unsigned long long average = nodeToDump->m_stats.m_callCount == 0 ? 0 : nodeToDump->m_stats.m_totalCycles / static_cast<unsigned long long>(nodeToDump->m_stats.m_callCount);
			nodeJson["4) Average cycles"] = average;

			nodeJson["5) Max cycles"] = nodeToDump->m_stats.m_maxCycles;
			nodeJson["6) Min cycles"] = nodeToDump->m_stats.m_minCycles;

			float percentage = nodeToDump->m_parent->m_stats.m_totalCycles == 0 ? 100.0f : (100.0f * static_cast<float>(nodeToDump->m_stats.m_totalCycles) / nodeToDump->m_parent->m_stats.m_totalCycles);
			nodeJson["7) % with respect to parent"] = percentage;

			// Save the stats of each child node in "childStats" and add it to "stats" as an element of an array
			ProfilingMgr::node* sibTraverser = nodeToDump->m_child;
			while (sibTraverser)
			{
				json childStats;
				dump_node(childStats, sibTraverser);
				nodeJson["8) Children"].push_back(childStats);

				sibTraverser = sibTraverser->m_sibling;
			}
		}
#endif


#if IMGUI_OUTPUT
		void ImGuiFormatter::on_gui()
		{
			if (!ImGui::Begin("PROFILER DATA (per frame)"))
			{
				ImGui::End();
				return;
			}

			ProfilingMgr::node* root = Profiler::ProfilingMgr::get_instance().get_root();

			dump_node(root->m_child);

			ImGui::End();
		}

		void ImGuiFormatter::dump_node(ProfilingMgr::node* nodeToDump)
		{
			if (nodeToDump == nullptr)
				return;

			if (ImGui::CollapsingHeader(nodeToDump->m_id))
			{
				ImGui::Text("Call count: %u", nodeToDump->m_stats.m_callCount);
				ImGui::Text("Total cycles: %llu", nodeToDump->m_stats.m_totalCycles);
				unsigned long long average = nodeToDump->m_stats.m_callCount == 0 ? 0 : nodeToDump->m_stats.m_totalCycles / static_cast<unsigned long long>(nodeToDump->m_stats.m_callCount);
				ImGui::Text("Avg cycles per call: %llu", average);
				ImGui::Text("Max cycles: %llu", nodeToDump->m_stats.m_maxCycles);
				ImGui::Text("Min cycles: %llu", nodeToDump->m_stats.m_minCycles);
				float percentage = nodeToDump->m_parent->m_stats.m_totalCycles == 0 ? 100.0f : (100.0f * static_cast<float>(nodeToDump->m_stats.m_totalCycles) / nodeToDump->m_parent->m_stats.m_totalCycles);
				ImGui::Text("%% with respect to parent: %f", percentage);

				//int valuesCount = static_cast<int>(std::clamp(nodeToDump->m_stats.m_callCount, 1u, CALLS_RECORDED));
				//int offset = m_valOffset++ % valuesCount;
				//ImGui::PlotLines("Cycles on previous calls", nodeToDump->m_stats.m_previousCycles, valuesCount, offset);
				//ImGui::Separator();
			}

			ProfilingMgr::node* sibTraverser = nodeToDump->m_child;
			while (sibTraverser)
			{
				ImGui::Indent(30.0f);
				dump_node(sibTraverser);
				sibTraverser = sibTraverser->m_sibling;
				ImGui::Unindent(30.0f);
			}
		}
#endif

	}
}

#endif	// USE_PROFILER