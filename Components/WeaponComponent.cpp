#include "StdAfx.h"
#include "WeaponComponent.h"

static void RegisterWeapon(Schematyc::IEnvRegistrar& registrar) {
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CWeaponComponent));
		// Functions
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterWeapon)

void CWeaponComponent::InitializeClass() {}

void CWeaponComponent::ProcessEventClass(SEntityEvent & event) {

	switch (event.event) {
	case ENTITY_EVENT_COLLISION:
		
		//Meele weapon specific
		//If weapon is meele
		if (GetWeaponProperties()->bIsMeele) {
			if (pOwnerEntity) {
				EventPhysCollision *physCollison = reinterpret_cast<EventPhysCollision *>(event.nParam[0]);
				if (physCollison) {

					//This Entity
					IPhysicalEntity *pThisEntityPhyics = physCollison->pEntity[0];
					IEntity *pThisEntity = gEnv->pEntitySystem->GetEntityFromPhysics(pThisEntityPhyics);
					IPhysicalEntity *pColliderPhysics = physCollison->pEntity[1];
					IEntity *pColliderEntity = gEnv->pEntitySystem->GetEntityFromPhysics(pColliderPhysics);

					//Gets the colliding entitys health and removes health from it if it's not dead
					if (pColliderEntity && pColliderEntity != m_pEntity && pColliderEntity != pOwnerEntity) {
						if (CHealthComponent *pVictimHealth = pColliderEntity->GetComponent<CHealthComponent>()) {
							if (pVictimHealth->IsAlive() && bIsAttacking) {
								bIsAttacking = false;
								pVictimHealth->Add((-GetWeaponProperties()->fDamage));
							}
						}
					}
				}
			}
		}

		break;

	case ENTITY_EVENT_TIMER:

		//Meele weapon specific
		//If weapon is meele
		if (GetWeaponProperties()->bIsMeele) {
			if (event.nParam[0] == Timer_Attack)
				bIsAttacking = false;
		}
		break;
	}

}

void CWeaponComponent::ReflectType(Schematyc::CTypeDesc<CWeaponComponent>& desc) {

	desc.SetGUID("{77818A15-58D9-4C70-9E53-B9EFD445F13C}"_cry_guid);
	desc.AddBase<SItemComponent>();
	desc.SetEditorCategory("Items");
	desc.SetLabel("Weapon Component");
	desc.SetDescription("Handles all the weapon logic");
	desc.AddMember(&CWeaponComponent::sItemProperties, 'itep', "ItemProperties", "Item Properties", "Sets all of the weapon properties", SItemProperties());
	desc.AddMember(&CWeaponComponent::sWeaponProperties, 'weap', "WeaponProperties", "Weapon Properties", "All of the different propeties for the weapon", SWeaponProperties());

}

//Shoots weapon if it is non-meele
void CWeaponComponent::Shoot() {

	if (GetWeaponProperties()->bIsMeele) {

		//!Add shooting here!

	}
}

//Starting meele attack
void CWeaponComponent::StartAttack() {

	bIsAttacking = true;
	m_pEntity->SetTimer(Timer_Attack, 1000);

}

//Stopping meele attack
void CWeaponComponent::StopAttack() {

	bIsAttacking = false;
	m_pEntity->KillTimer(Timer_Attack);

}