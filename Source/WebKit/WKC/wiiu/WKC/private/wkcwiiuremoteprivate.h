/*
 *  wkcwiiuremoteprivate.h
 *
 *  Copyright(c) 2012 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef _WKC_WIIU_REMOTE_PRIVATE_H_
#define _WKC_WIIU_REMOTE_PRIVATE_H_

#include <wkc/wkcbase.h>
#include "wkcwiiuremoteproc.h"

namespace WKC {

void WKCWiiuRemoteUpdate( int in_id, WKCWiiuRemote* inout_data );
void WKCWiiuRemotesetCursorViewable( int in_id, bool in_viewable );

}

#endif // _WKC_WIIU_REMOTE_PRIVATE_H_
