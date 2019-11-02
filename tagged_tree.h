#ifndef TAGGED_TREE_H
#define TAGGED_TREE_H

#include <map>
#include <memory>
#include <vector>

template<typename T>
class TaggedTree {
public:
	class Vertex {
		friend class TaggedTree;
		const size_t depth;
		std::map<size_t, std::unique_ptr<Vertex>> children;
		Vertex* parent;
		const size_t child_no;
		bool tagged;
		Vertex(size_t depth, Vertex* parent, size_t child_no, T data, bool tagged) : depth(depth), parent(parent), child_no(child_no), tagged(tagged), data(std::move(data)) {}
	public:
		T data;
		Vertex() = delete;
		Vertex(const Vertex&) = delete;
		Vertex(Vertex&&) = default;
		Vertex* grow(T data) {
			return children.emplace(children.size(), std::unique_ptr<Vertex>(new Vertex(depth + 1, this, children.size(), std::move(data), true))).first->second.get();
		}
		void untag(TaggedTree& tree){
			tagged = false;
			if(children.empty()) {
				Vertex* current_vertex = this;
				while(current_vertex && (!current_vertex->tagged) && current_vertex->children.empty()) {
					int v = *current_vertex->data.value;
					std::cout<<v<<'a'<<std::endl;
					Vertex* current_parent = current_vertex->parent;
					if(!current_parent) {
						tree.root = nullptr;
						current_vertex = nullptr;
					} else {
						current_parent->children.erase(current_vertex->child_no);
						current_vertex = current_parent;
					}
					std::cout<<v<<'b'<<std::endl;
				}
			}
			std::cout<<'c'<<std::endl;
			while(tree.root && tree.root->children.size() == 1 && !tree.root->tagged) {
				tree.root = std::move(tree.root->children.begin()->second);
				if(tree.root) {
					tree.root->parent = nullptr;
				}
			}
			std::cout<<'d'<<std::endl;
		}
		std::vector<const Vertex*> path_to(const Vertex& target) const {
			const Vertex* first = this;
			const Vertex* second = &target;
			std::vector<const Vertex*> this_path;
			std::vector<const Vertex*> that_path;
			while(first->depth > second->depth) {
				this_path.push_back(first);
				first = first->parent;
			}
			while(second->depth > first->depth) {
				that_path.push_back(second);
				second = second->parent;
			}
			while(first != second) {
				this_path.push_back(first);
				first = first->parent;
				that_path.push_back(second);
				second = second->parent;
			}
			this_path.push_back(first);
			for(auto i = that_path.crbegin(); i != that_path.crend(); ++i) {
				this_path.push_back(*i);
			}
			return this_path;
		}
	};
private:
	friend class Vertex;
	std::unique_ptr<Vertex> root;
	TaggedTree() = default;
	TaggedTree(const TaggedTree&) = delete;
	TaggedTree(TaggedTree&&) = delete;
public:
	static std::pair<std::unique_ptr<TaggedTree>, Vertex*> make_tree(T data) {
		auto tree = std::unique_ptr<TaggedTree>(new TaggedTree());
		tree->root = std::unique_ptr<Vertex>(new Vertex(0, nullptr, 0, std::move(data), true));
		return std::make_pair(std::move(tree), tree->root.get());
	}
};

#endif
