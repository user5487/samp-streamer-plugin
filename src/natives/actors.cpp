/*
 * Copyright (C) 2016 Incognito
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../natives.h"

#include "../core.h"
#include "../utility.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <Eigen/Core>

cell AMX_NATIVE_CALL Natives::CreateDynamicActor(AMX *amx, cell *params)
{
	CHECK_PARAMS(13, "CreateDynamicActor");
	if (core->getData()->getGlobalMaxItems(STREAMER_TYPE_ACTOR) == core->getData()->actors.size())
	{
		return 0;
	}
	int actorID = Item::Actor::identifier.get();
	Item::SharedActor actor(new Item::Actor);
	actor->amx = amx;
	actor->actorID = actorID;
	actor->inverseAreaChecking = false;
	actor->originalComparableStreamDistance = -1.0f;
	actor->modelID = static_cast<int>(params[1]);
	actor->position = Eigen::Vector3f(amx_ctof(params[2]), amx_ctof(params[3]), amx_ctof(params[4]));
	actor->rotation = amx_ctof(params[5]);
	actor->invulnerable = static_cast<int>(params[6]) != 0;
	actor->health = amx_ctof(params[7]);
	Utility::addToContainer(actor->worlds, static_cast<int>(params[8]));
	Utility::addToContainer(actor->interiors, static_cast<int>(params[9]));
	Utility::addToContainer(actor->players, static_cast<int>(params[10]));
	actor->comparableStreamDistance = amx_ctof(params[11]) < STREAMER_STATIC_DISTANCE_CUTOFF ? amx_ctof(params[11]) : amx_ctof(params[11]) * amx_ctof(params[11]);
	actor->streamDistance = amx_ctof(params[11]);
	Utility::addToContainer(actor->areas, static_cast<int>(params[12]));
	actor->priority = static_cast<int>(params[13]);
	core->getGrid()->addActor(actor);
	core->getData()->actors.insert(std::make_pair(actorID, actor));
	return static_cast<cell>(actorID);
}

cell AMX_NATIVE_CALL Natives::DestroyDynamicActor(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "DestroyDynamicActor");
	boost::unordered_map<int, Item::SharedActor>::iterator a = core->getData()->actors.find(static_cast<int>(params[1]));
	if (a != core->getData()->actors.end())
	{
		Utility::destroyActor(a);
		return 1;
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::IsValidDynamicActor(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "IsValidDynamicActor");
	boost::unordered_map<int, Item::SharedActor>::iterator a = core->getData()->actors.find(static_cast<int>(params[1]));
	if (a != core->getData()->actors.end())
	{
		return 1;
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::ApplyDynamicActorAnimation(AMX *amx, cell *params)
{
	CHECK_PARAMS(9, "ApplyDynamicActorAnimation");
	boost::unordered_map<int, Item::SharedActor>::iterator a = core->getData()->actors.find(static_cast<int>(params[1]));
	if (a != core->getData()->actors.end())
	{
		a->second->anim = boost::intrusive_ptr<Item::Actor::Anim>(new Item::Actor::Anim);
		a->second->anim->lib = Utility::convertNativeStringToString(amx, params[2]);
		a->second->anim->name = Utility::convertNativeStringToString(amx, params[3]);
		a->second->anim->delta = amx_ctof(params[4]);
		a->second->anim->loop = static_cast<int>(params[5]) != 0;
		a->second->anim->lockx = static_cast<int>(params[6]) != 0;
		a->second->anim->locky = static_cast<int>(params[7]) != 0;
		a->second->anim->freeze = static_cast<int>(params[8]) != 0;
		a->second->anim->time = static_cast<int>(params[9]);
		boost::unordered_map<int, int>::iterator i = core->getData()->internalActors.find(a->first);
		if (i != core->getData()->internalActors.end())
		{
			sampgdk::ApplyActorAnimation(i->second, a->second->anim->lib.c_str(), a->second->anim->name.c_str(), a->second->anim->delta, a->second->anim->loop, a->second->anim->lockx, a->second->anim->locky, a->second->anim->freeze, a->second->anim->time);
		}
		return 1;
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::ClearDynamicActorAnimations(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "ClearDynamicActorAnimations");
	boost::unordered_map<int, Item::SharedActor>::iterator a = core->getData()->actors.find(static_cast<int>(params[1]));
	if (a != core->getData()->actors.end())
	{
		a->second->anim = NULL;
		boost::unordered_map<int, int>::iterator i = core->getData()->internalActors.find(a->first);
		if (i != core->getData()->internalActors.end())
		{
			sampgdk::ClearActorAnimations(i->second);
		}
		return 1;
	}
	return 0;
}

cell AMX_NATIVE_CALL Natives::GetPlayerTargetDynamicActor(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "GetPlayerTargetDynamicActor");
	boost::unordered_map<int, Player>::iterator p = core->getData()->players.find(static_cast<int>(params[1]));
	if (p != core->getData()->players.end())
	{
		int actorid = sampgdk::GetPlayerTargetActor(p->second.playerID);
		for (boost::unordered_map<int, int>::iterator i = core->getData()->internalActors.begin(); i != core->getData()->internalActors.end(); ++i) {
			if (i->second == actorid) {
				return i->first;
			}
		}
	}
	return 0;
}
