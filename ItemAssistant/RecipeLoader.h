#pragma once
#include "stdafx.h"
#include <Shared/UnicodeSupport.h>
#include <list>
#include <vector>
#include "RecipeList.h"
#include "shared/Mutex.h"
#include "shared/Thread.h"
#include "shared/aopackets.h"
#include <shared\IDB.h>

class RecipeLoader {
public:
	RecipeLoader(sqlite::IDBPtr db);
	std::vector<std::tstring> getRecipes() const;
	const RecipeList* const getRecipe(const std::tstring& recipe) const;

private:
	std::vector<RecipeList> _recipes;

	std::set<unsigned int> LoadRecipe(const std::tstring& filename);
	std::list<std::tstring> getRecipeFilenames() const;
};