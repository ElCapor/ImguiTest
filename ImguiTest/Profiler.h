/**
* @file Profiler.h
* @author Miguel Echeverria , 540000918 , miguel.echeverria@digipen.edu
* @date 2021/03/23
* @brief Contains the declaration of the Profiler class which contains the
*		 statistics gathered of the program where it has been used.
*
* @copyright Copyright (C) 2020 DigiPen Institute of Technology .
*/

#pragma once

// IMPORTANT: Set this macro to 1/0 to enable/disable the profiler
#define USE_PROFILER 1


#if USE_PROFILER
// Client must use this macros so that code still compiles when undefining USE_PROFILER
#define SCOPED_PROFILER(nameId) Profiler::ScopedProfiler prof##__LINE__(nameId);			// Scoped profiler where the user can specify an ID
#define FUNCTION_PROFILER()		Profiler::ScopedProfiler prof##__LINE__(__FUNCSIG__);		// Scoped profiler that uses the signature of the function we are in as the ID
#define PROF_NEW_FRAME()		Profiler::ProfilingMgr::get_instance().new_frame();
#define PROF_SET_ACTIVE(active) Profiler::ProfilingMgr::get_instance().setProfilerActive(active);
#define PROF_GET_ACTIVE()		Profiler::ProfilingMgr::get_instance().getProfilerActive();

namespace Profiler
{
	class ScopedProfiler
	{
	public:

		// Default ctor. Records the cycles passed since the CPU started.
		ScopedProfiler(const char * id);

		// Dtor. Records the cycles passed since the CPU started. Subtracts this
		// with the constructor recording to get the cycles that have passed since
		// construction of this object.
		~ScopedProfiler();

	private:
	};


	// This will be the ammount of calls to each function whose stats will be recorded, for graph plotting purposes.
	//const unsigned CALLS_RECORDED = 10;

	class ProfilingMgr
	{
	public:

		struct node_stats
		{
			node_stats();

			// Resets all the stats to 0 or their default value.
			void reset();

			unsigned m_recursionLevel;
			unsigned m_callCount;
			unsigned long long m_startCycles;
			unsigned long long m_totalCycles;
			unsigned long long m_maxCycles;
			unsigned long long m_minCycles;
			//float m_previousCycles[CALLS_RECORDED] = {0};
		};

		struct node
		{
			node(const char* id = nullptr, node* parent = nullptr, node* children = nullptr, node* sibling = nullptr);

			// Helper function that returns the child node of parent with the same id as passed.
			// If no child exists with that id, returns nullptr.
			node* find_child_node(const char* id) const;

			// Adds a child node (as m_child if it is null, or at the end of siblings otherwise).
			void add_child(node* child);


			const char* m_id = nullptr;
			node* m_parent = nullptr;
			node* m_child = nullptr;
			node* m_sibling = nullptr;

			node_stats m_stats;
		};


		// Dtor. Frees the memory of the tree.
		~ProfilingMgr();

		// Retrieves the only instance of Profiler
		static ProfilingMgr& get_instance();


		// Gets called when a block of code to be profiled with id passed as parameter is entered and starts profiling it.
		void enter(const char * id);

		// Gets called when the current block of code that is being profiled exits, and records statistics about the number
		// of calls, cycles passed etc.
		void exit();

		// Marks the start of a frame. As a result, resets all the statistics of all the nodes.
		void new_frame();

		// Getter and setter for the flag that indicates whether the profiler is active or not.
		bool getProfilerActive();
		void setProfilerActive(bool active);

		// Return the root node of the tree.
		node* get_root() const;

		// Return the current node of the tree.
		node* get_current_node() const;

	private:

		node* m_root = nullptr;									// Root node of the tree (ID = "Root")
		node* m_currentNode = nullptr;							// Represents the current function the program is executing

		bool  m_profilerActive = true;

		// Helper function to allocate a node of the tree with a specific id.
		node* create_node(const char * id) const;

		// Helper function to free the memory of a specific node.
		void free_node(node* nodeToFree) const;

		// Helper function to free the memory of all the nodes in a specific tree.
		void free_tree(node* treeToFree) const;

		// Reset the stats of all the nodes in the tree passed as parameter.
		void reset_tree_stats(node* treeToReset) const;

		ProfilingMgr();											// Default ctor. Private because of singleton pattern
		ProfilingMgr(const ProfilingMgr&) = delete;				// Copy ctor deleted because of singleton pattern
		ProfilingMgr& operator=(const ProfilingMgr&) = delete;	// Assignment operator deleted because of singleton pattern
	};

}

#else

// Empty versions of the scoped profiling macros so that the user code still compiles when deactivating the profiler
#define SCOPED_PROFILER(nameId)
#define FUNCTION_PROFILER()
#define PROF_NEW_FRAME()
#define PROF_SET_ACTIVE(active)
#define PROF_GET_ACTIVE()

#endif	// USE_PROFILER