#include "StdAfx.h"
#include "WeaponComponent.h"
#include <CryNetwork\Rmi.h>

using ServerShootRMI = SRmi<RMI_WRAP(&CWeaponComponent::ServerShoot)>;
using ClientShootRMI = SRmi<RMI_WRAP(&CWeaponComponent::ClientShoot)>;

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

void CWeaponComponent::InitializeClass() {

	//Shoot RMI(Server)
	{
		const bool bIsServer = true;
		const ERMIAttachmentType attachmentType = eRAT_NoAttach;
		const ENetReliabilityType reliability = eNRT_UnreliableOrdered;

		ServerShootRMI::Register(this, attachmentType, bIsServer, reliability);
	}

	//Shoot RMI(Client)
	{
		const bool bIsServer = false;
		const ERMIAttachmentType attachmentType = eRAT_NoAttach;
		const ENetReliabilityType reliability = eNRT_UnreliableOrdered;

		ClientShootRMI::Register(this, attachmentType, bIsServer, reliability);
	}

}

void CWeaponComponent::ProcessEventClass(const SEntityEvent & event) {

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
	desc.AddMember(&CWeaponComponent::sFiremodeProperties, 'feap', "FiremodeProperties", "Firemode Properties", "Sets all of the firemode properties", SFiremodeProperties());

}

//Reloads the current weapon
void CWeaponComponent::Reload() {

	if (iCurrAmmo == 0) {
		//Remove the amount of ammo in a full clip from max ammo
		GetWeaponProperties()->iMaxAmmo -= GetWeaponProperties()->iClipAmmo;
		//Sets current ammo to the max clip size
		iCurrAmmo = GetWeaponProperties()->iClipAmmo;
	}
	else if (iCurrAmmo >= 1) {
		//Get the value between max clip ammo and current ammo
		int iNewAmmo = GetWeaponProperties()->iClipAmmo - iCurrAmmo;
		//Remove the vaule above from max ammo
		GetWeaponProperties()->iMaxAmmo -= iNewAmmo;
		//Sets current ammo to the max clip size
		iCurrAmmo = GetWeaponProperties()->iClipAmmo;
	}
	else if (iCurrAmmo < 0) {
		//if current ammo is less than zero, set it back to zero
		iCurrAmmo = 0;
	}
}

//Shoots the weapon
void CWeaponComponent::Shoot() {

	if (!GetWeaponProperties()->bIsMeele) {
		if (Cry::DefaultComponents::CAdvancedAnimationComponent *pAnimationComponent = pPlayerShooting->GetEntity()->GetComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>()) {
			if (ICharacterInstance *pCharacter = pAnimationComponent->GetCharacter()) {

				auto *pBarrelOutAttachment = pCharacter->GetIAttachmentManager()->GetInterfaceByName("barrel_out");

				if (pBarrelOutAttachment != nullptr) {

					QuatTS bulletOrigin = pBarrelOutAttachment->GetAttWorldAbsolute();
					SEntitySpawnParams spawnParams;

					spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
					spawnParams.vPosition = bulletOrigin.t;
					spawnParams.qRotation = bulletOrigin.q;

					const float bulletScale = 0.05f;
					spawnParams.vScale = Vec3(bulletScale);

					//Spawn the actual entity
					if (IEntity *pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams)) {

						//Adds Bullet Component to Entity
						pEntity->CreateComponentClass<CBulletComponent>();
						if (CBulletComponent *pBullet = pEntity->GetComponent<CBulletComponent>())
							pBullet->SetPlayer(pPlayerShooting);

						//Remove bullet from mag
						iCurrAmmo -= 1;

					}

				}

			}
		}

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

//////////////////Network//////////////////

//Called by the server
bool CWeaponComponent::ClientShoot(SShootParams && p, INetChannel * pNetChannel) {

	//Check so that weapon acually isn't meele
	if (!GetWeaponProperties()->bIsMeele) {
		if (Cry::DefaultComponents::CAdvancedAnimationComponent *pAnimationComponent = pPlayerShooting->GetEntity()->GetComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>()) {
			if (ICharacterInstance *pCharacter = pAnimationComponent->GetCharacter()) {

				auto *pBarrelOutAttachment = pCharacter->GetIAttachmentManager()->GetInterfaceByName("barrel_out");

				if (pBarrelOutAttachment != nullptr) {

					QuatTS bulletOrigin = pBarrelOutAttachment->GetAttWorldAbsolute();
					SEntitySpawnParams spawnParams;

					spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
					spawnParams.vPosition = bulletOrigin.t;
					spawnParams.qRotation = bulletOrigin.q;

					const float bulletScale = 0.05f;
					spawnParams.vScale = Vec3(bulletScale);

					//Spawn the actual entity
					if (IEntity *pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams)) {

						//Adds Bullet Component to Entity
						pEntity->CreateComponentClass<CBulletComponent>();
						if (CBulletComponent *pBullet = pEntity->GetComponent<CBulletComponent>())
							pBullet->SetPlayer(pPlayerShooting);

						//Remove bullet from mag
						iCurrAmmo -= 1;

					}

				}

			}
		}

	}

	return true;
}

//Called by RequestShot
bool CWeaponComponent::ServerShoot(SShootParams && p, INetChannel * pNetChannel) {
	//Shoots on all the clients
	ClientShootRMI::InvokeOnAllClients(this, SShootParams{ p.playerId });

	return true;
}

//Called when player presses fire button
bool CWeaponComponent::RequestShot(EntityId playerId) {
	if (gEnv->bServer) {



	}
	else {
		//Sends request to server
		ServerShootRMI::InvokeOnServer(this, SShootParams{ playerId });
	}

	return false;
}