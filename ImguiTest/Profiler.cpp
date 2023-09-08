/**
* @file Profiler.cpp
* @author Miguel Echeverria , 540000918 , miguel.echeverria@digipen.edu
* @date 2021/03/23
* @brief Contains the implementation of the Profiler.
*
* @copyright Copyright (C) 2020 DigiPen Institute of Technology .
*/


#include "Profiler.h"

#if USE_PROFILER
#include <intrin.h>		// __rdtsc
#include <limits>		// std::numeric_limits



namespace Profiler
{
	// Default ctor. Records the cycles passed since the CPU started.
	ScopedProfiler::ScopedProfiler(const char* id)
	{
		ProfilingMgr::get_instance().enter(id);
	}

	// Dtor. Records the cycles passed since the CPU started. Subtracts this
	// with the constructor recording to get the cycles that have passed since
	// construction of this object.
	ScopedProfiler::~ScopedProfiler()
	{
		ProfilingMgr::get_instance().exit();
	}



	// Retrieves the only instance of Profiler
	ProfilingMgr& ProfilingMgr::get_instance()
	{
		static ProfilingMgr instance;
		return instance;
	}


	// Default ctor. Creates the root node.
	ProfilingMgr::ProfilingMgr()
	{
		m_root = create_node("Root");
		m_currentNode = m_root;
	}

	// Dtor. Frees the memory of the tree.
	ProfilingMgr::~ProfilingMgr()
	{
		free_tree(m_root);
		m_currentNode = nullptr;
	}


	// Gets called when a block of code to be profiled with id passed as parameter is entered and starts profiling it.
	void ProfilingMgr::enter(const char* id)
	{
		if (!m_profilerActive)
			return;

		// If recursive function, increase the recursion level
		if (m_currentNode->m_id == id)
		{
			m_currentNode->m_stats.m_recursionLevel++;
			return;
		}

		// Otherwise check to see if this id already exists as a child of the current node
		node* child = m_currentNode->find_child_node(id);

		// If it doesn't exist, create a node for it
		if (child == nullptr)
		{
			child = create_node(id);
			m_currentNode->add_child(child);
		}

		m_currentNode = child;

		// Increase the call count and record the CPU cycles
		child->m_stats.m_callCount++;
		child->m_stats.m_startCycles = __rdtsc();
	}


	// Gets called when the current block of code that is being profiled exits, and records statistics about the number
	// of calls, cycles passed etc.
	void ProfilingMgr::exit()
	{
		if (!m_profilerActive)
			return;

		// If it's a recursive function, simply reduce the recursion level
		if (m_currentNode->m_stats.m_recursionLevel > 0)
		{
			m_currentNode->m_stats.m_recursionLevel--;
			return;
		}
		
		// Otherwise, record CPU cycles and return to parent
		unsigned long long cyclesTaken = __rdtsc() - m_currentNode->m_stats.m_startCycles;

		// Record on the array of samples the cycles taken for the current call of this node's scope/function
		//m_currentNode->m_stats.m_previousCycles[m_currentNode->m_stats.m_callCount % CALLS_RECORDED] = static_cast<float>(cyclesTaken);

		// Update the maximum and minimum number of cycles
		if (cyclesTaken > m_currentNode->m_stats.m_maxCycles)
			m_currentNode->m_stats.m_maxCycles = cyclesTaken;
		if (cyclesTaken < m_currentNode->m_stats.m_minCycles)
			m_currentNode->m_stats.m_minCycles = cyclesTaken;

		m_currentNode->m_stats.m_totalCycles += cyclesTaken;
		m_currentNode = m_currentNode->m_parent;
	}


	// Marks the start of a frame. As a result, resets all the statistics of all the nodes.
	void ProfilingMgr::new_frame()
	{
		if (!m_profilerActive)
			return;

		reset_tree_stats(m_root);
	}


	// Getter and setter for the flag that indicates whether the profiler is active or not.
	bool ProfilingMgr::getProfilerActive()
	{
		return m_profilerActive;
	}
	void ProfilingMgr::setProfilerActive(bool active)
	{
		m_profilerActive = active;
	}


	// Return the root node of the tree.
	ProfilingMgr::node* ProfilingMgr::get_root() const
	{
		return m_root;
	}

	// Return the current node of the tree.
	ProfilingMgr::node* ProfilingMgr::get_current_node() const
	{
		return m_currentNode;
	}


	ProfilingMgr::node_stats::node_stats()
		:	m_recursionLevel(0),
			m_callCount(0),
			m_startCycles(0),
			m_totalCycles(0)
	{
		m_maxCycles = 0;
		m_minCycles = std::numeric_limits<unsigned long long>::max();
	}

	// Resets all the stats to 0 or their default value.
	void ProfilingMgr::node_stats::reset()
	{
		m_recursionLevel = 0;
		m_callCount = 0;
		m_startCycles = 0;
		m_totalCycles = 0;
		m_maxCycles = 0;
		m_minCycles = std::numeric_limits<unsigned long long>::max();
	}


	ProfilingMgr::node::node(const char* id, node* parent, node* children, node* sibling)
		:	m_id(id),
			m_parent(parent),
			m_child(children),
			m_sibling(sibling)
	{
	}


	// Helper function that returns the child node (or sibling of child) of this with the same id as passed.
	// If no child exists with that id, returns nullptr.
	ProfilingMgr::node* ProfilingMgr::node::find_child_node(const char* id) const
	{
		// If the child is null, we already know it doesn't exist
		if (m_child == nullptr)
			return nullptr;

		// Check if the child is the same
		if (m_child->m_id == id)
			return m_child;

		// Traverse through the siblings of the children to find it
		node* traverser = m_child->m_sibling;
		while (traverser)
		{
			if (traverser->m_id == id)
				return traverser;

			traverser = traverser->m_sibling;
		}

		return nullptr;
	}

	// Adds a child node (as m_child if it is null, or at the end of siblings otherwise).
	void ProfilingMgr::node::add_child(node* child)
	{
		child->m_parent = this;

		if (m_child == nullptr)
		{
			m_child = child;
			return;
		}

		node* sibTraverser = m_child;
		while (sibTraverser->m_sibling)
		{
			sibTraverser = sibTraverser->m_sibling;
		}

		sibTraverser->m_sibling = child;
	}



	// Helper function to allocate a node of the tree with a specific id.
	ProfilingMgr::node* ProfilingMgr::create_node(const char* id) const
	{
		return new node(id);
	}


	// Helper function to free the memory of a specific node.
	void ProfilingMgr::free_node(ProfilingMgr::node* nodeToFree) const
	{
		delete nodeToFree;
		nodeToFree = nullptr;
	}


	// Helper function to free the memory of all the nodes in a specific tree.
	void ProfilingMgr::free_tree(ProfilingMgr::node* treeToFree) const
	{
		// If we reached a null node, return
		if (treeToFree == nullptr)
			return;

		// Delete the nodes using post-order traversal
		free_tree(treeToFree->m_child);
		free_tree(treeToFree->m_sibling);
		free_node(treeToFree);
	}


	// Reset the stats of all the nodes in the tree passed as parameter.
	void ProfilingMgr::reset_tree_stats(node* treeToReset) const
	{
		if (treeToReset == nullptr)
			return;

		reset_tree_stats(treeToReset->m_child);
		reset_tree_stats(treeToReset->m_sibling);
		treeToReset->m_stats.reset();
	}

}

#endif	// USE_PROFILER