/*
 * Copyright (C) 2011-2016 Red Hat, Inc.
 * (C) Copyright 2015-2016 Hewlett Packard Enterprise Development LP
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: tasleson
 *         Joe Handzik <joseph.t.handzik@hpe.com>
 *         Gris Ge <fge@redhat.com>
 */

#ifndef LSM_CAPABILITIES_H
#define LSM_CAPABILITIES_H

#include "libstoragemgmt_common.h"

#ifdef  __cplusplus
extern "C" {
#endif

/** @file libstoragemgmt_capabilities.h*/

/*Note: Domain is 0..255 */
/** \enum lsm_capability_value_type Possible values for supported feature*/
typedef enum {
    LSM_CAP_UNSUPPORTED = 0,             /**< Feature is not supported */
    LSM_CAP_SUPPORTED = 1                /**< Feature is supported */
} lsm_capability_value_type;

/** \enum lsm_capability_value_type Capabilities supported by array */
typedef enum {
    /** List volumes */
    LSM_CAP_VOLUMES = 20,
    /** Create volumes */
    LSM_CAP_VOLUME_CREATE = 21,
    /** Resize volumes */
    LSM_CAP_VOLUME_RESIZE = 22,

    /** Replication is supported */
    LSM_CAP_VOLUME_REPLICATE = 23,
    /** Can make a space efficient copy of volume */
    LSM_CAP_VOLUME_REPLICATE_CLONE = 24,
    /** Can make a bitwise copy of volume */
    LSM_CAP_VOLUME_REPLICATE_COPY = 25,
    /** Mirror data with delay */
    LSM_CAP_VOLUME_REPLICATE_MIRROR_ASYNC = 26,
    /** Mirror data and always in sync */
    LSM_CAP_VOLUME_REPLICATE_MIRROR_SYNC = 27,

    /** Size of a block for range operations */
    LSM_CAP_VOLUME_COPY_RANGE_BLOCK_SIZE = 28,
    /** Sub volume replication support */
    LSM_CAP_VOLUME_COPY_RANGE = 29,
    /** Can space efficient copy a region(s) of a volume*/
    LSM_CAP_VOLUME_COPY_RANGE_CLONE = 30,
    /** Can copy a region(s) of a volume */
    LSM_CAP_VOLUME_COPY_RANGE_COPY = 31,

    /** Can delete a volume */
    LSM_CAP_VOLUME_DELETE = 33,

    /** Enable volume*/
    LSM_CAP_VOLUME_ENABLE = 34,
    /** Disable volume*/
    LSM_CAP_VOLUME_DISABLE = 35,

    /** Grant an access group to a volume */
    LSM_CAP_VOLUME_MASK = 36,
    /** Revoke access for an access group */
    LSM_CAP_VOLUME_UNMASK = 37,
    /** List access groups */
    LSM_CAP_ACCESS_GROUPS = 38,
    /** Create an access group */
    LSM_CAP_ACCESS_GROUP_CREATE_WWPN = 39,
    /** Delete an access group */
    LSM_CAP_ACCESS_GROUP_DELETE = 40,
    /** Add an initiator to an access group */
    LSM_CAP_ACCESS_GROUP_INITIATOR_ADD_WWPN = 41,
    /** Remove an initiator from an access group */
    LSM_CAP_ACCESS_GROUP_INITIATOR_DELETE = 42,

    /** Retrieve a list of volumes accessible by an access group */
    LSM_CAP_VOLUMES_ACCESSIBLE_BY_ACCESS_GROUP = 43,
    /** Retrieve a list of what access groups are accessible for a given
     * volume */
    LSM_CAP_ACCESS_GROUPS_GRANTED_TO_VOLUME = 44,


    /** Used to determine if a volume has any dependencies */
    LSM_CAP_VOLUME_CHILD_DEPENDENCY = 45,
    /** Removes dependendies */
    LSM_CAP_VOLUME_CHILD_DEPENDENCY_RM = 46,

    /** Create iSCSI access group */
    LSM_CAP_ACCESS_GROUP_CREATE_ISCSI_IQN = 47,
    /** For empty access group, this indicates it can add iSCSI IQN to it */
    LSM_CAP_ACCESS_GROUP_INITIATOR_ADD_ISCSI_IQN = 48,

    /** If you can configure iSCSI chap authentication */
    LSM_CAP_VOLUME_ISCSI_CHAP_AUTHENTICATION = 53,

    /** ^ If you can query RAID information from volume */
    LSM_CAP_VOLUME_RAID_INFO = 54,

    /** Thin provisioned volumes are supported */
    LSM_CAP_VOLUME_THIN = 55,

    /** List file systems */
    LSM_CAP_FS = 100,
    /** Delete a file system */
    LSM_CAP_FS_DELETE = 101,
    /** Resize a file system */
    LSM_CAP_FS_RESIZE = 102,
    /** Create a file system */
    LSM_CAP_FS_CREATE = 103,
    /** Clone a file system */
    LSM_CAP_FS_CLONE = 104,
    /** Clone a file on a file system */
    LSM_CAP_FILE_CLONE = 105,
    /** List FS snapshots */
    LSM_CAP_FS_SNAPSHOTS = 106,
    /** Create a snapshot */
    LSM_CAP_FS_SNAPSHOT_CREATE = 107,
    /** Delete a snapshot */
    LSM_CAP_FS_SNAPSHOT_DELETE = 109,
    /** Revert the state of a FS to the specified snapshot */
    LSM_CAP_FS_SNAPSHOT_RESTORE = 110,
    /** Revert the state of a list of files to a specified snapshot */
    LSM_CAP_FS_SNAPSHOT_RESTORE_SPECIFIC_FILES = 111,
    /** Determine if a child dependency exists for the specified file */
    LSM_CAP_FS_CHILD_DEPENDENCY = 112,
    /** Remove any dependencies the file system may have */
    LSM_CAP_FS_CHILD_DEPENDENCY_RM = 113,
    /** Remove any dependencies for specific files */
    LSM_CAP_FS_CHILD_DEPENDENCY_RM_SPECIFIC_FILES = 114,

    /** Get a list of supported client authentication types */
    LSM_CAP_EXPORT_AUTH = 120,
    /** List exported file systems */
    LSM_CAP_EXPORTS = 121,
    /** Export a file system */
    LSM_CAP_EXPORT_FS = 122,
    /** Remove an export */
    LSM_CAP_EXPORT_REMOVE = 123,
    /** Plug-in allows user to define custome export path */
    LSM_CAP_EXPORT_CUSTOM_PATH = 124,

    /** Plug-in allows user to change system read cache percentage */
    LSM_CAP_SYS_READ_CACHE_PCT_UPDATE = 158,
    /** Plug-in allows user to retrieve system read cache percentage */
    LSM_CAP_SYS_READ_CACHE_PCT_GET = 159,
    /** Plug-in allows user to retrieve storage firmware version */
    LSM_CAP_SYS_FW_VERSION_GET = 160,
    /** Plug-in allows user to retrieve storage mode*/
    LSM_CAP_SYS_MODE_GET = 161,
    /** Plug-in allows user to retrieve disk location */
    LSM_CAP_DISK_LOCATION = 163,
    /** Plug-in allows user to retrieve disk rotation speed */
    LSM_CAP_DISK_RPM = 164,
    /** Plug-in allows user to retrieve disk link type */
    LSM_CAP_DISK_LINK_TYPE = 165,
    /** Plugin allows user to enable and disable volume LEDs */
    LSM_CAP_VOLUME_LED = 171,
    /** Seach occurs on array */
    LSM_CAP_POOLS_QUICK_SEARCH = 210,
    /** Seach occurs on array */
    LSM_CAP_VOLUMES_QUICK_SEARCH = 211,
    /** Seach occurs on array */
    LSM_CAP_DISKS_QUICK_SEARCH = 212,
    /** Seach occurs on array */
    LSM_CAP_ACCESS_GROUPS_QUICK_SEARCH = 213,
    /** Seach occurs on array */
    LSM_CAP_FS_QUICK_SEARCH = 214,
    /** Seach occurs on array */
    LSM_CAP_NFS_EXPORTS_QUICK_SEARCH = 215,

    /** List target ports */
    LSM_CAP_TARGET_PORTS = 216,
    /** Filtering occurs on array */
    LSM_CAP_TARGET_PORTS_QUICK_SEARCH = 217,

    /** List disk drives */
    LSM_CAP_DISKS = 220,
    /** Query pool member information */
    LSM_CAP_POOL_MEMBER_INFO = 221,

    /** Create RAID volume */
    LSM_CAP_VOLUME_RAID_CREATE = 222,

    /** Query SCSI VPD83 ID of disk */
    LSM_CAP_DISK_VPD83_GET = 223,

} lsm_capability_type;

/**
 * Free the memory used by the storage capabilities data structure
 * @param cap   Valid storage capability data structure.
 * @return Error code as enumerated by \ref lsm_error_number.
 * @retval LSM_ERR_OK on success.
 */
int LSM_DLL_EXPORT lsm_capability_record_free(lsm_storage_capabilities *cap);

/**
 * Return the capability for the specified feature.
 * @param cap   Valid pointer to capability data structure
 * @param t     Which capability you are interested in
 * @return Value of supported enumerated type.
 */
lsm_capability_value_type LSM_DLL_EXPORT
    lsm_capability_get(lsm_storage_capabilities * cap,
                       lsm_capability_type t);

/**
 * Boolean version of capability support
 * @param cap
 * @param t
 * @return Non-zero if supported, 0 if not supported
 */
int LSM_DLL_EXPORT lsm_capability_supported(lsm_storage_capabilities *cap,
                                            lsm_capability_type t);


#ifdef  __cplusplus
}
#endif
#endif
