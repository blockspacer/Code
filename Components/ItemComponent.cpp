#include "StdAfx.h"
#include "ItemComponent.h"
#include <CrySchematyc\Env\Elements\EnvComponent.h>

 void SItemComponent::Initialize() {
	 
	//Load basic item stuff
	LoadGeometry();
	Physicalize();
	CreateItemName();
	InitializeClass();
}
 
 uint64 SItemComponent::GetEventMask() const {
	 return ENTITY_EVENT_BIT(ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED) | ENTITY_EVENT_BIT(ENTITY_EVENT_COLLISION) | ENTITY_EVENT_BIT(ENTITY_EVENT_TIMER);
}
 
 void SItemComponent::ProcessEvent(const SEntityEvent & event) {
	 
	switch (event.event) {
	case ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED:
	{
				 
		if (sItemProperties != sPrevItemProperties) {
					 
			sPrevItemProperties = sItemProperties;
					 
			LoadGeometry();
			Physicalize();	 
		}
	}
	break;		 
	}
	ProcessEventClass(event);
}
 
 void SItemComponent::ReflectType(Schematyc::CTypeDesc<SItemComponent>& desc) {
	 
	desc.SetGUID("{A77F1211-9E03-4697-9E04-DC35F6E15B89}"_cry_guid);

}
 
 void SItemComponent::LoadGeometry() {
	 
	string sGeomPath = GetProperties()->sRenderProperties.sGeomPath.value.c_str();
	 
	if (sGeomPath.empty())
		return;
	 
	m_pEntity->LoadGeometry(GEOMETRY_SLOT, sGeomPath);
	 	 
}
 
 void SItemComponent::Physicalize() {
	 
	SEntityPhysicalizeParams physParams;
	physParams.mass = GetProperties()->sPhysicsProperties.fMass;
	physParams.type = PE_RIGID;
	m_pEntity->Physicalize(physParams);
		 
}
 
 void SItemComponent::CreateItemName() {
	 
	string sLongName = m_pEntity->GetClass()->GetName();

	sLongName.erase(0, sLongName.find_last_of(':') + 1);
	string sFirst;
	sFirst.SetAt(0, sLongName[0]);
	sFirst.MakeUpper();
	sLongName.erase(0, 1);
	GetProperties()->sItemName = sFirst + sLongName;
	 
		 
}

 void SItemComponent::PickUp(IEntity *pNewOwner) {

	 if (!pNewOwner)
		 return;

	 pOwnerEntity = pNewOwner;
	 pOwnerEntity->AttachChild(m_pEntity);


	 //filter collision

	 pe_action_add_constraint constraint;
	 constraint.pt[0] = ZERO;
	 constraint.flags = constraint_ignore_buddy | constraint_inactive;
	 constraint.pBuddy = pOwnerEntity->GetPhysicalEntity();
	 iChildConstraintId = m_pEntity->GetPhysicalEntity()->Action(&constraint);

	 //add collision filtering to owner

	 constraint.flags |= constraint_inactive;
	 constraint.pBuddy = m_pEntity->GetPhysicalEntity();
	 iOwnerConstraintId = pOwnerEntity->GetPhysicalEntity()->Action(&constraint);

 }

 void SItemComponent::Drop(SItemComponent *pWeaponToDrop) {
	 
	if (!pOwnerEntity)
		return;
	
	//If it owner entity has PlayerComponent, continue
	if (CPlayerComponent *pOwnerPlayer = pOwnerEntity->GetComponent<CPlayerComponent>()) {
		//Detach the weapon from players hand
		pOwnerPlayer->GetInventory()->DetachFromHand();

		//Get the weapon slot
		int iWeaponSlot = pOwnerPlayer->GetInventory()->GetWeaponSlot(pWeaponToDrop);
		//Erase the weapon from the index
		pOwnerPlayer->GetInventory()->RemoveWeapon(iWeaponSlot);
	}
	
	 
	//Stop ignoring owner collision
	pe_action_update_constraint constraintUpdate;
	constraintUpdate.bRemove = 1;
	constraintUpdate.idConstraint = iChildConstraintId;
	m_pEntity->GetPhysicalEntity()->Action(&constraintUpdate);
	 
	constraintUpdate.idConstraint = iOwnerConstraintId;
	pOwnerEntity->GetPhysicalEntity()->Action(&constraintUpdate);
	 
	pe_action_impulse impulse;
	impulse.impulse = pOwnerEntity->GetForwardDir();
	 
	m_pEntity->GetPhysicalEntity()->Action(&impulse);
	 
	pOwnerEntity = nullptr; 
}
 
 bool SItemComponent::IsPickable() {
	 
	 return !pOwnerEntity;	 
}
