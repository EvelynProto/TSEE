#include "../tsee.h"

/**
 * @brief Check if a specific attribute is set in an attribute.
 *
 * @param attr Attribute to check.
 * @param against Attribute to check against.
 * @return true if it is set.
 */
bool TSEE_Attributes_Check(TSEE_Object_Attributes attr,
						   TSEE_Object_Attributes against) {
	if (against == TSEE_ATTRIB_NONE) return attr = against;
	else return attr & against;
}

/**
 * @brief Sets an attribute on an attribute object
 *
 * @param attr Attribute to modify
 * @param to_set Attribute to set
 */
void TSEE_Attributes_Set(TSEE_Object_Attributes *attr,
						 TSEE_Object_Attributes to_set) {
	if (to_set == TSEE_ATTRIB_NONE) *attr = TSEE_ATTRIB_NONE;
	else *attr |= to_set;
}