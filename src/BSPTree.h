#pragma once

#include "BSPNode.h"
#include "BoundingBox.h"
#include "IPrim.h"
#include "ray.h"

namespace {
	// Calculates and return the bounding box, containing the whole scene
	CBoundingBox calcBoundingBox(const std::vector<ptr_prim_t>& vpPrims)
	{
		CBoundingBox res;
		// --- PUT YOUR CODE HERE ---
		for (auto pPrim : vpPrims)
			res.extend(pPrim->getBoundingBox());
		return res;
	}

	// Returns the best dimension index for next split
	int MaxDim(const Vec3f& v)
	{
		return (v.val[0] > v.val[1]) ? ((v.val[0] > v.val[2]) ? 0 : 2) : ((v.val[1] > v.val[2]) ? 1 : 2);
	}
}

// ================================ BSP Tree Class ================================
/**
 * @brief Binary Space Partitioning (BSP) tree class
 */
class CBSPTree
{
public:
	CBSPTree(void) = default;
	CBSPTree(const CBSPTree&) = delete;
	~CBSPTree(void) = default;
	const CBSPTree& operator=(const CBSPTree&) = delete;
	
	/**
	 * @brief Builds the BSP tree for the primitives provided via \b vpPrims
	 * @param vpPrims The vector of pointers to the primitives in the scene
	 * @param maxDepth The maximum allowed depth of the tree.
	 * Increasing the depth of the tree may speed-up rendering, but increse the memory consumption.
	 * @param minPrimitives The minimum number of primitives in a leaf-node.
	 * This parameters should be alway above 1.
	 */
	void build(const std::vector<ptr_prim_t>& vpPrims, size_t maxDepth = 20, size_t minPrimitives = 3) {
		m_treeBoundingBox = calcBoundingBox(vpPrims);
		m_maxDepth = maxDepth;
		m_minPrimitives = minPrimitives;
		std::cout << "Scene bounds are : " << m_treeBoundingBox << std::endl;
		m_root = build(m_treeBoundingBox, vpPrims, 0);
	}
	/**
	 * @brief Checks whether the ray \b ray intersects a primitive.
	 * @details If ray \b ray intersects a primitive, the \b ray.t value will be updated
	 * @param[in,out] ray The ray
	 */
	bool intersect(Ray& ray) const
	{
		// --- PUT YOUR CODE HERE ---
		double t0 = 0;
		double t1 = ray.t;
		m_treeBoundingBox.clip(ray, t0, t1);
		if (t1 < t0) return false;  // no intersection with the bounding box

		return m_root->intersect(ray, t0, t1);
	}


private:
	/**
	 * @brief Builds the BSP tree
	 * @details This function builds the BSP tree recursively
	 * @param box The bounding box containing all the scene primitives
	 * @param vpPrims The vector of pointers to the primitives included in the bounding box \b box
	 * @param depth The distance from the root node of the tree
	 */
	ptr_bspnode_t build(const CBoundingBox& box, const std::vector<ptr_prim_t>& vpPrims, size_t depth)
	{
		// Check for stoppong criteria
		if (depth >= m_maxDepth || vpPrims.size() <= m_minPrimitives)
			return std::make_shared<CBSPNode>(vpPrims);                                     // => Create a leaf node and break recursion

		// else -> prepare for creating a branch node
		// First split the bounding volume into two halfes
		int     splitDim = MaxDim(box.getMaxPoint() - box.getMinPoint());                   // Calculate split dimension as the dimension where the aabb is the widest
		float   splitVal = (box.getMinPoint()[splitDim] + box.getMaxPoint()[splitDim]) / 2; // Split the aabb exactly in two halfes
		auto    splitBoxes = box.split(splitDim, splitVal);
		CBoundingBox& lBox = splitBoxes.first;
		CBoundingBox& rBox = splitBoxes.second;

		// Second order the primitives into new nounding boxes
		std::vector<ptr_prim_t> lPrim;
		std::vector<ptr_prim_t> rPrim;
		for (auto pPrim : vpPrims) {
			if (pPrim->getBoundingBox().overlaps(lBox))
				lPrim.push_back(pPrim);
			if (pPrim->getBoundingBox().overlaps(rBox))
				rPrim.push_back(pPrim);
		}

		// Next build recursively 2 subtrees for both halfes
		auto pLeft = build(lBox, lPrim, depth + 1);
		auto pRight = build(rBox, rPrim, depth + 1);

		return std::make_shared<CBSPNode>(splitDim, splitVal, pLeft, pRight);
	}

	
private:
	CBoundingBox 	m_treeBoundingBox;		///<
	size_t			m_maxDepth;				///< The maximum allowed depth of the tree
	size_t			m_minPrimitives;		///< The minimum number of primitives in a leaf-node
	ptr_bspnode_t   m_root = nullptr;		///<
};
	
