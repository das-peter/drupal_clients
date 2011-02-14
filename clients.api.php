<?php
// $Id$

/**
 * @file
 * Hooks provided by the Clients module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Inform Clients about connection types.
 *
 * @return array
 *   An array of information on the connection types implemented by a module,
 *   keyed by the machine-readable name for the type.
 *   Each type is itself an array, with following keys:
 *     'module' => the current module
 *     'label'  => The human-readable label.
 */
function hook_imagecache_actions() {
    return array(
      'my_client' => array(
        'module' => 'my_client_module',
        'label'  => t('My Client Type'),
      ),
    );
  }
}
