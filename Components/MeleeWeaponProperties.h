#pragma once

#include "StdAfx.h"

struct SMeeleWeaponProperties {

	inline bool operator==(const SMeeleWeaponProperties& rhs) const { return 0 == memcmp(this, &rhs, sizeof(rhs)); }
	inline bool operator!=(const SMeeleWeaponProperties& rhs) const { return 0 != memcmp(this, &rhs, sizeof(rhs)); }

	float fDamage = 0.f;

};

static void ReflectType(Schematyc::CTypeDesc<SMeeleWeaponProperties>& desc) {

	desc.SetGUID("{CEC4F200-D21E-456E-ACD7-282B90EED08B}"_cry_guid);
	desc.SetLabel("Meele Weapon Properties");
	desc.SetDescription("Meele Weapon Properties");
	desc.AddMember(&SMeeleWeaponProperties::fDamage, 'fdam', "Damage", "Damage", "Damage amount", 0.f);

}
