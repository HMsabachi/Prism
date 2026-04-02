#pragma once

#include "Core.h"
#include "Layer.h"

#include <vector>

namespace Prism 
{
	/// <summary>
	/// LayerStack 是一个逻辑上的双段容器，底层由 std::vector 实现。
	/// 它将“普通层(Layers)”存储在前半部分，将“覆盖层(Overlays)”存储在后半部分。
	/// 通过维护 m_LayerInsert 迭代器，新层会被插入到普通层区域的末尾，而覆盖层则始终添加至容器最末尾。
	/// 这种结构确保了渲染时从头到尾遍历可实现正确的遮盖顺序，同时支持从后往前遍历以实现 UI 优先的事件分发。
	/// LayerStack is a logically two-part container, implemented using `std::vector`.
	/// It stores regular layers(`Layers`) in the first half and overlays(`Overlays`) in the second half.
	///	By maintaining an `m_LayerInsert` iterator, new layers are inserted at the end of the regular layer area, while overlays are always added to the very end of the container.
	///	This structure ensures correct overlay order during rendering by traversing from beginning to end, while also supporting back - to - forefront traversal for UI - priority event dispatching.
	/// </summary>
	class PRISM_API LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end() const { return m_Layers.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};

}
