#pragma once

#include "deps/rapidxml/rapidxml.hpp"
#include "deps/rapidxml/rapidxml_print.hpp"

namespace engine {
namespace n_safe_file {
//

struct Value {
	enum type_e : u16 {
		NO_VALUE=0,

		STRING,

		INT_, IV2, IV3, IV4,
		FLT, FV2, FV3, FV4,
		BOOL, BV2, BV3, BV4,

		//SRGB8, SRGBA8,
		//LRF, LRGF, LRGBF, LRGBAF,
	};
	enum mode_e : u16 {
		DIMENSIONLESS=0,
		ANGLE, // in this struct angles are stores as radiants, but are converted on write out to file and read in
	};

	type_e	type = NO_VALUE;
	mode_e	mode = DIMENSIONLESS;

	union {
		std::string		str		; // is this ok, is this string just in an invalid state after default construction of Value

		int				int_	;
		iv2				iv2_	;
		iv3				iv3_	;
		iv4				iv4_	;

		flt				flt_	;
		v2				v2_		;
		v3				v3_		;
		v4				v4_		;

		bool			bool_	;
		bv2				bv2_	;
		bv3				bv3_	;
		bv4				bv4_	;

		//srgb8			srgb8_;
		//srgba8			srgba8_;
		// LRF - LRGBAF just map to flt_ - v4_ here
	};

	static type_e get_type (std::string* val) { return STRING	; }
	
	static type_e get_type (int		   * val) { return INT_		; }
	static type_e get_type (iv2		   * val) { return IV2		; }
	static type_e get_type (iv3		   * val) { return IV3		; }
	static type_e get_type (iv4		   * val) { return IV4		; }
	
	static type_e get_type (flt		   * val) { return FLT		; }
	static type_e get_type (v2		   * val) { return FV2		; }
	static type_e get_type (v3		   * val) { return FV3		; }
	static type_e get_type (v4		   * val) { return FV4		; }
	
	static type_e get_type (bool	   * val) { return BOOL		; }
	static type_e get_type (bv2		   * val) { return BV2		; }
	static type_e get_type (bv3		   * val) { return BV3		; }
	static type_e get_type (bv4		   * val) { return BV4		; }

	Value () {}
	~Value () {
		using std::string;
		// placement destruct union
		if (type == STRING)
			str.~string(); // placement destruct

	}

	void assign (void* val, Value::type_e val_type, Value::mode_e val_mode) {
		if ((type == STRING) != (val_type == STRING)) { // either was string and is not anymore or the reverse
			
			this->~Value();

			using std::string;
			// placement construct union
			if (val_type == STRING)
				new (&str)string(); // placement new
		}
		type = val_type;
		mode = val_mode;

		switch (type) {
			case STRING	:	str		= *(std::string	*)val;	return;

			case INT_	:	int_	= *(int			*)val;	return;
			case IV2	:	iv2_	= *(iv2			*)val;	return;
			case IV3	:	iv3_	= *(iv3			*)val;	return;
			case IV4	:	iv4_	= *(iv4			*)val;	return;

			case FLT	:	flt_	= *(flt			*)val;	return;
			case FV2	:	v2_		= *(v2			*)val;	return;
			case FV3	:	v3_		= *(v3			*)val;	return;
			case FV4	:	v4_		= *(v4			*)val;	return;

			case BOOL	:	bool_	= *(bool		*)val;	return;
			case BV2	:	bv2_	= *(bv2			*)val;	return;
			case BV3	:	bv3_	= *(bv3			*)val;	return;
			case BV4	:	bv4_	= *(bv4			*)val;	return;

				//SRGB8, SRGBA8,
				//LRF, LRGF, LRGBF, LRGBAF,

			default: assert(not_implemented); return;
		}
	}

	std::string print (int const* v, int count) {
		std::string str;
		for (int i=0; i<count; ++i)
			prints(&str, "%d%s", v[i], i == (count -1) ? "":", ");
		return str;
	}

	std::string print (flt const* v, int count) {
		std::string str;
		for (int i=0; i<count; ++i)
			if (mode == ANGLE)
				prints(&str, "%g deg%s", to_deg(v[i]), i == (count -1) ? "":", ");
			else
				prints(&str, "%g%s", v[i], i == (count -1) ? "":", ");
		return str;
	}

	std::string print (bool const* v, int count) {
		std::string str;
		for (int i=0; i<count; ++i)
			prints(&str, "%s%s", v[i] ? "true":"false", i == (count -1) ? "":", ");
		return str;
	}

	std::string print () {
		switch (type) {
			case STRING	:	return str;
			
			case INT_	:	return print( &int_		, 1	);
			case IV2	:	return print( &iv2_.x	, 2	);
			case IV3	:	return print( &iv3_.x	, 3	);
			case IV4	:	return print( &iv4_.x	, 4	);
			
			case FLT	:	return print( &flt_		, 1	);
			case FV2	:	return print( &v2_.x	, 2	);
			case FV3	:	return print( &v3_.x	, 3	);
			case FV4	:	return print( &v4_.x	, 4	);
			
			case BOOL	:	return print( &bool_	, 1	);
			case BV2	:	return print( &bv2_.x	, 2	);
			case BV3	:	return print( &bv3_.x	, 3	);
			case BV4	:	return print( &bv4_.x	, 4	);

				//SRGB8, SRGBA8,
				//LRF, LRGF, LRGBF, LRGBAF,

			default: assert(not_implemented); return "--error--";
		}
	}
	
};

struct Node {
	std::string			name;

	Value				val;

	Node*				parent = nullptr;
	
	std::vector< unique_ptr<Node> >	children;

	Node (cstr name): name{name} {}

};

struct Save {
	std::string filepath;

	bool		trigger_load;
	bool		trigger_save;

	std::vector< unique_ptr<Node> >	root_children;

	Node*				cur_parent = nullptr;

	std::vector< unique_ptr<Node> >* get_children (Node* parent) {
		return parent ? &parent->children : &root_children;
	}

	Node* node_insert (Node* parent, int insert_indx, cstr name) {
		auto* parent_children = get_children(parent);
		
		auto up = make_unique<Node>(name);
		auto* n = up.get();

		n->parent = parent;
		parent_children->emplace(parent_children->begin() +insert_indx, std::move(up));
		
		return n;
	}

	Node* match_node (cstr name) {
		auto* cur_children = get_children(cur_parent);

		auto res = std::find_if(cur_children->begin(), cur_children->end(), [&] (unique_ptr<Node> const& n) { return n->name.compare(name) == 0; });
		if (res == cur_children->end())
			return nullptr; // node not found

		return res->get();
	}
	Node* match_array_node (cstr name, int indx) {
		auto* cur_children = get_children(cur_parent);

		if (!(indx >= 0 && indx < (int)cur_children->size()))
			return nullptr;

		auto* n = (*cur_children)[indx].get();

		if (n->name.compare(name) != 0)
			return nullptr;
		
		return n;
	}

	Node* _node (cstr name) {
		auto* n = match_node(name);
		if (!n) {
			n = node_insert(cur_parent, (int)get_children(cur_parent)->size(), name);
		}
		return n;
	}
	void _value (cstr name, void* val, Value::type_e val_type, Value::mode_e val_mode) {
		auto* n = _node(name);
		n->val.assign(val, val_type, val_mode);
	}

	// finds/inserts node at under current parent node (either root or nodes created by open begin() call)
	//  names are searched, so they must be unique (or else the prev node with that names will be overwritten)
	template <typename T> inline void value (cstr name, T* val) {
		_value(name, val, Value::get_type(val), Value::DIMENSIONLESS);
	}
	template <typename T> inline void angle (cstr name, T* val) { // same as value(), just specifies that values is an angle (can be a vector of angles)
		_value(name, val, Value::get_type(val), Value::ANGLE);
	}

	// like value(), but creates a value-less node and makes it the current parent (following value() nodes will be children of this node)
	//  must always close a begin() with and end()
	void begin (cstr name) {
		auto* n = _node(name);
		cur_parent = n;
	}
	void end () {
		if (!cur_parent)
			fprintf(stderr, "Save: too many end() calls!\n");
		else
			cur_parent = cur_parent->parent;
	}

	std::vector<int> cur_array_indx;

	// like begin() / end(), but children will be array members, which means that they all should have the same name and must also be one node per array element (begin() / end())
	//  on load: returns the number of subelements (so you can resize your array to that size and then load the values by calling begin() that many times) 
	//  else:	 return -1, increase size of array by calling begin() n times, if you call begin() less times than there are elements the remaining ones will be kept for now (could be removed in )
	int begin_array (cstr name) {
		begin(name);

		cur_array_indx.push_back(0);

		return -1;
	}
	void end_array () {
		int length = cur_array_indx.back();
		cur_array_indx.pop_back();

		get_children(cur_parent)->resize(length);

		end();
	}

	void begin_arr_elem (cstr name) {
		auto* n = match_array_node(name, cur_array_indx.back());
		if (!n) {
			n = node_insert(cur_parent, cur_array_indx.back(), name);
		}
		cur_parent = n;

		cur_array_indx.back()++;
	}
	void end_arr_elem () {
		end();
	}

	//
	void _node_to_xml_recurse (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* parent_xml_node, Node* node) {
		
		auto* value_str = node->val.type == Value::NO_VALUE ? "" : doc->allocate_string( node->val.print().c_str() );

		auto xml_node = doc->allocate_node(rapidxml::node_type::node_element, node->name.c_str(), value_str);
		parent_xml_node->append_node(xml_node);
		
		for (auto& n : node->children)
			_node_to_xml_recurse(doc, xml_node, n.get());
	}
	void end_frame () {
		if (cur_parent)
			fprintf(stderr, "Save: not few end() calls!\n");
		cur_parent = nullptr;

		if (trigger_save)
			to_xml();
	}

	void to_xml () {

		rapidxml::xml_document<>	xml_doc;

		for (auto& n : root_children)
			_node_to_xml_recurse(&xml_doc, &xml_doc, n.get());

		struct Print_Iterator {
			std::string*	text;
			int				ptr;

			Print_Iterator (std::string* text_) {
				text = text_;
				text->assign(1, '\0');
				ptr = 0;
			}

			// these increment operators invalidate the result of operator* (dereference), but unincremented versions of Print_Iterator are still valid
			// increment operators allocate enough space in the text (with '\0'), so that a dereference on the resulting Iterator is always valid, NOTE: if the user does a *it++ = for the last char, there will be two null terminators
			Print_Iterator operator++ (int) {
				auto ret = *this; // copy this unincremented

				text->push_back('\0');
				++ptr;

				return ret;
			}
			Print_Iterator operator++ () {
				text->push_back('\0');
				++ptr;

				return *this;
			}

			char& operator* () const { // dereference
				return (*text)[ptr];
			}
		};

		std::string text;
		Print_Iterator it(&text);

		rapidxml::print(it, xml_doc, 0);

		if (text.size() > 0 && text[text.size() -1] == '\0')
			text.resize(text.size() -1); // remove redundant null terminator

		write_fixed_size_binary_file((filepath +".xml").c_str(), text.c_str(), text.size());
	}
};

Save* save_file (cstr filepath, bool trigger_load, bool trigger_save) {
	trigger_load = trigger_load && !trigger_save; // dont attempt to load and save at the same time

	static Save s;
	if (s.filepath.size() == 0) {
		s.filepath = filepath;
	} else {
		assert(s.filepath.compare(filepath) == 0);
	}

	s.trigger_load = trigger_load;
	s.trigger_save = trigger_save;

	return &s;
}

//
}
using n_safe_file::Save;
using n_safe_file::save_file;
}
