#include "ResourceManager.h"
#include <blib/Resource.h>
#include <blib/util/Log.h>

#include <assert.h>
using blib::util::Log;

namespace blib
{
	ResourceManager* ResourceManager::manager = nullptr;

	void ResourceManager::cleanup()
	{
		for (auto r : toDelete)
		{
#ifdef _DEBUG
			Log::out << "Deleting " << r->name << Log::newline;
#endif
			delete r;
		}
		toDelete.clear();
	}

	ResourceManager::ResourceManager()
	{
		manager = this;
	}


	ResourceManager::~ResourceManager()
	{
		for (auto it : resources)
			delete it.first;
	}

	ResourceManager& ResourceManager::getInstance()
	{
		return *manager;
	}

	Resource* ResourceManager::regResource(Resource* resource)
	{
		if (resources.find(resource) == resources.end())
			resources[resource] = 0;
		resources[resource]++;
      //  Log::out<<"Loading "<<resource->name<<Log::newline;
		return resource;
	}


	void ResourceManager::dispose(Resource* resource)
	{
		assert(resources.find(resource) != resources.end());
		resources[resource]--;
		if (resources[resource] == 0)
		{
#ifdef _DEBUG
			Log::out<<"Marking "<<resource->name<<Log::newline;
#endif
			toDelete.push_back(resource);

			resources.erase(resources.find(resource));
		}
	}


	void ResourceManager::printDebug()
	{
		for (auto r : resources)
		{
			if (r.second > 0)
			{
				Log::out << r.first->name << " still loaded " << r.second << " times" << Log::newline;
#ifdef RESOURCECALLSTACK
				Log::out << r.first->callstack << Log::newline;
#endif
			}
		}
	}

}