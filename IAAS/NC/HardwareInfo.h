/*
 * HardwareInfo.h
 *
 *  Created on: 2011-11-3
 *      Author: lgy
 */

#ifndef HARDWAREINFO_H_
#define HARDWAREINFO_H_
#include <libvirt/libvirt.h>
#include <string.h>

#include "../common/String.h"

class HardwareInfo
{
protected:
	int width_analyse(char*);
public:
	int getCPUcount();
	int getRamSize();
	int getDiskSize();
	int getNetWidth();
	String getVirtType();
};


#endif /* HARDWAREINFO_H_ */
