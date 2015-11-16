
// ==================================================================
// @(#)ht1632.h
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 28/07/2014
// $Id$
//
// libbuspirate
// Copyright (C) 2014 Bruno Quoitin
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
// 02111-1307  USA
// ==================================================================

#ifndef __BUSPIRATE_HT1632_H__
#define __BUSPIRATE_HT1632_H__

#include <buspirate.h>

int ht1632_init(BP * bp);
int ht1632_command(BP * bp, unsigned char cmd);
int ht1632_write(BP * bp, unsigned char addr, unsigned char data);

int demo_ht1632(BP * bp, int argc, char * argv[]);

#endif /* __BUSPIRATE_HT1632_H__ */
