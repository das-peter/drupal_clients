<?php
// $Id$
/**
 * @file
 * Actual function bodies for default hook definitions in
 * feeds_defaults.features.inc.
 */

/**
 * Helper to implementation of hook_ctools_plugin_api().
 */
function _clients_feeds_ctools_plugin_api() {
  $args = func_get_args();
  $module = array_shift($args);
  $api = array_shift($args);
  if ($module == "data" && $api == "data_default") {
    return array("version" => 1);
  }
  else if ($module == "feeds" && $api == "feeds_importer_default") {
    return array("version" => 1);
  }
}


/**
 * Helper to implementation of hook_feeds_importer_default().
 */
function _clients_feeds_feeds_importer_default() {
  
  $export = array();
  if (module_exists('clients_feeds')) {
    $feeds_importer = new stdClass;
    $feeds_importer->disabled = TRUE; /* Edit this to true to make a default feeds_importer disabled initially */
    $feeds_importer->api_version = 1;
    $feeds_importer->id = 'clients_feeds'; //??
    $feeds_importer->config = array(
      'name' => 'Clients feed',
      'description' => 'Create nodes from clients resources.',
      'fetcher' => array(
        'plugin_key' => 'FeedsClientsFetcher',
        'config' => array(),
      ),
      'parser' => array(
        'plugin_key' => 'FeedsClientsParser',
        'config' => array(
          'delimiter' => ',',
        ),
      ),
      'processor' => array(
        'plugin_key' => 'FeedsNodeProcessor',
        'config' => array(
          'content_type' => 'story',
          'update_existing' => 1,
          'expire' => '-1',
          'mappings' => array(
            '0' => array(
              'source' => 'title',
              'target' => 'title',
              'unique' => FALSE,
            ),
          ),
        ),
      ),
      'content_type' => '',
      'update' => 0,
      'import_period' => '-1',
      'expire_period' => 3600,
      'import_on_create' => 1,
    );
  
    $export['clients_feeds'] = $feeds_importer;
  }
  return $export;
}
