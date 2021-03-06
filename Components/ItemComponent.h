/*-------------------------------------------------------------------------------

Author : Ivar J�nsson
Project : BR
Purpose : Base structure for items. All types of items will derive from here.

-------------------------------------------------------------------------------*/

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include "ItemProperties.h"
#include "CryNetwork\Rmi.h"

struct SItemComponent : public IEntityComponent {

	struct SPickUpParams {

		EntityId Id;

		void SerializeWith(TSerialize ser) {
			ser.Value("EntityId", Id, 'eid');
		}

	};

	SItemComponent() = default;
	virtual ~SItemComponent() = default;

	virtual void Initialize() override;
	virtual void InitializeClass() = 0;
	virtual uint64 GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual void ProcessEventClass(const SEntityEvent& event) = 0;
	static void ReflectType(Schematyc::CTypeDesc<SItemComponent>& desc);
	//geometry and physics
	virtual void LoadGeometry();
	virtual void Physicalize();

	virtual SItemProperties *GetProperties() { return &sItemProperties; }
	virtual void CreateItemName();
	virtual string GetItemName() { return GetProperties()->sItemName;  }

	virtual EItemType GetItemType() { return GetProperties()->eItemType; }
	virtual int GetItemLevel() { return GetProperties()->itemLevel; }
	virtual float GetItemWeight() { return GetProperties()->fItemWeight; }

	virtual void Drop(SItemComponent *pWeaponToDrop);
	virtual bool IsPickable();

	//Network
	bool ServerPickup(SPickUpParams&& p, INetChannel* pNetChannel);
	bool ClientPickup(SPickUpParams&& p, INetChannel* pNetChannel);
	void RequestPickup(IEntity *pNewOwner);
	//Network
protected:
	SItemProperties sItemProperties, sPrevItemProperties;

	//Item owners, after and before pickup
	IEntity *pOwnerEntity = nullptr;
	IEntity *pNewOwner = nullptr;

	EntityId pickUpItemId;

	int iChildConstraintId = 0;
	int iOwnerConstraintId = 0;

};