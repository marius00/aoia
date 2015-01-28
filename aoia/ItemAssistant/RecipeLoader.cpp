#include "stdafx.h"
#include "DBManager.h"
#include "RecipeLoader.h"
#include <windows.h>
#include <set>

#include <boost/spirit.hpp>
#include <boost/spirit/include/classic_tree_to_xml.hpp>
#define TIXML_USE_STL
#include <tinyxml.h>
#include <iostream>
#include "Shlwapi.h"

#include <boost/signal.hpp>
#include <shared/IDB.h>
#include <settings/ISettings.h>

RecipeLoader::RecipeLoader(sqlite::IDBPtr db) {
	auto recipes = getRecipeFilenames();

	std::set<unsigned int> ids;
	for (auto it : recipes) {
		auto ids_ = LoadRecipe(it);
		ids.insert(ids_.cbegin(), ids_.cend());
	}

	
	if (!ids.empty()) {
		boolean first = true;
		std::tstringstream st;
		st << _T("SELECT aoid, name FROM tblAo WHERE aoid in (");
		for (auto it : ids) {
			if (!first)
				st << _T(",");
			st << it;
			first = false;
		}
		st << _T(")");



		g_DBManager.Lock();
		sqlite::ITablePtr itemnames = db->ExecTable(st.str());
		g_DBManager.UnLock();
		
		// Extract names
		std::map<unsigned int, std::tstring> nameMap;
		if (itemnames != NULL) {
			for (unsigned int i = 0; i < itemnames->Rows(); i++) {
				unsigned int id = boost::lexical_cast<unsigned int>(itemnames->Data(i, 0));
				const std::string nameAscii = itemnames->Data(i, 1);
				std::tstring name;				
				if (!nameAscii.empty()) {
					name = from_ascii_copy(nameAscii);
				}
				else 
					name = _T("?");

				nameMap[id] = name;
			}
		}

		// Update the "Item"'s
		for (int i = 0; i < _recipes.size(); i++) {
			_recipes[i].updateNames(nameMap);
		}
	}
}

std::vector<std::tstring> RecipeLoader::getRecipes() const {
	std::vector<std::tstring> recipes;
	for (auto it = _recipes.cbegin(); it != _recipes.cend(); it++)
		recipes.push_back(it->getName());

	return recipes;
}
const RecipeList* const RecipeLoader::getRecipe(const std::tstring& recipe) const {
	for (auto it = _recipes.cbegin(); it != _recipes.cend(); it++) {
		if (recipe.compare(it->getName()) == 0)
			return &(*it);
	}

	return NULL;
}
std::set<unsigned int> RecipeLoader::LoadRecipe(const std::tstring& filename) {
	if (!PathFileExists((_T("lists\\") + filename).c_str()))
		return std::set<unsigned int>();

	TiXmlDocument document;
	std::set<Item> items;
	std::set<Combo> combos;
	std::set<ItemCopy> copies;
	std::tstring recipeName;
	std::set<unsigned int> ids;

	if (document.LoadFile(to_ascii_copy(_T("lists\\") + filename).c_str(), TIXML_ENCODING_UTF8)) {
		TiXmlHandle docHandle( &document );
		
		// Load items
		TiXmlElement* element = docHandle.FirstChild( "Recipe" ).FirstChild( "Items" ).FirstChild().Element();
		while (element) {
			if (std::string("Item").compare(element->Value()) == 0) {
				Item item;
				
				element->QueryUnsignedAttribute("id", &item.id);
				element->QueryUnsignedAttribute("count", &item.count);
				element->QueryUnsignedAttribute("minql", &item.minql);
				element->QueryUnsignedAttribute("maxql", &item.maxql);
				const char* t = element->GetText();
				if (t) {
					item.comments = boost::spirit::xml::encode(from_ascii_copy(t));
				}
				items.insert(item);
				ids.insert(item.id);
			}
			element = element->NextSiblingElement();
		}

		// Load combos
		element = docHandle.FirstChild( "Recipe" ).FirstChild( "Combos" ).FirstChild().Element();
		while (element) {
			if (std::string("Combo").compare(element->Value()) == 0) {
				Combo combo;

				element->QueryUnsignedAttribute("id", &combo.id);
				element->QueryUnsignedAttribute("child1", &combo.childred[0]);
				element->QueryUnsignedAttribute("child2", &combo.childred[1]);
				element->QueryUnsignedAttribute("minql", &combo.minql);
				element->QueryUnsignedAttribute("maxql", &combo.maxql);
				combos.insert(combo);
				//ids.insert(combo.id);
			}
			element = element->NextSiblingElement();

		}

		// Load copies / duplicates
		element = docHandle.FirstChild( "Recipe" ).FirstChild( "Copies" ).FirstChild().Element();
		while (element) {
			if (std::string("Copy").compare(element->Value()) == 0) {
				ItemCopy icopy;

				element->QueryUnsignedAttribute("id", &icopy.id);
				element->QueryUnsignedAttribute("copy", &icopy.newId);
				copies.insert(icopy);
				//ids.insert(combo.id);
			}
			element = element->NextSiblingElement();

		}

		element = docHandle.FirstChild( "Recipe" ).FirstChild( "Description" ).Element();
		if (element != NULL) {
			const char* name = element->GetText();
			if (name != NULL)
				recipeName = boost::spirit::xml::encode(from_ascii_copy(name));
			else
				recipeName = filename;
		} 
		else {
			recipeName = filename;
		}

		RecipeList recipe(recipeName, items, combos, copies);
		if (!recipe.isValid())
			return std::set<unsigned int>();
		else
			_recipes.push_back(recipe);
	}

	return ids;
}


std::list<std::tstring> RecipeLoader::getRecipeFilenames() const {
	std::list<std::tstring> result;

	std::wstring path = L"lists\\*";
	WIN32_FIND_DATA findData;

	HANDLE hFind = FindFirstFileEx(path.c_str(), FindExInfoStandard, &findData, FindExSearchLimitToDirectories, NULL, 0);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (
				(findData.cFileName[0] != NULL) 
				&& (findData.cFileName[0] != '.'))
			{
				result.push_back(std::tstring(findData.cFileName));
			}
		}
		while (FindNextFile(hFind, &findData));
		FindClose(hFind);
	}

	return result;
}
