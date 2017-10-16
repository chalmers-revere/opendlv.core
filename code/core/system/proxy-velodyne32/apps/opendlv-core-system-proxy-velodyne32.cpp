/**
 * proxy-velodyne32 - Interface to HDL-32E.
 * Copyright (C) 2017 Hang Yin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ProxyVelodyne32.h"

int32_t main(int32_t argc, char **argv) {
    opendlv::core::system::proxy::ProxyVelodyne32 velodyne32(argc, argv);
    return velodyne32.runModule();
}
