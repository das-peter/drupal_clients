<?php
// $Id$
/**
 * @file
 * Using devseed extendr module as a base... 
 * http://devseed.svn.cvsdude.com/sandbox/drupal-6/extendr/
 */

/**
 * For now this is an extension of the views_query, I'm not sure if it should
 * remain so.
 */
class clientsQuery extends views_plugin_query_default {

  var $args = array();
  var $method = ''; // possibly remove?

  function init($table, $field) {
    $this->args['extras'] = array();
    $this->method = ''; // don't think default method applies to clients
  }

  /**
   * Generate queries
   *
   * - Fields don't matter here, they are dealt with purely at the display level.
   *
   *  @param $get_count
   *    We don't use this at all as the flickr api returns this automatically.
   */
  function query($get_count = FALSE) { }

  /**
   * @todo make this lie less.
   */
  function ensure_table() {
    return TRUE;
  }

  function set_method($m, $a) {
    $this->method = $m;
    $this->args += $a;
  }

  /**
   * @todo Investigate adding fields dynamically from response - e.g. remote cck.
   *
   * function add_field($table, $field, $alias = '', $params = NULL) {
   * $extras = array(
   *    'date_upload' => 'dateupload',
   *  );
   *  if (array_key_exists($field, $extras)) {
   *    $this->args['extras'][] = $field;
   *    $alias = $extras[$field];
   *  }
   *  return parent::add_field($table, $field, $alias, $params);
   * }
   */

  /**
   * Execute a call
   */
  function execute(&$view) {

    $this->args['extras'] = implode(',', $this->args['extras']);

    if (!empty($view->pager['items_per_page'])) {
      $this->args['per_page'] = $view->pager['items_per_page'];
    }
    else {
      $this->args['per_page'] = 500; // upper limit?
    }

    // What page was requested:
    $pager_page_array = isset($_GET['page']) ? explode(',', $_GET['page']) : array();
    if (!empty($pager_page_array[$view->pager['element']])) {
      $page = intval($pager_page_array[$view->pager['element']]);
      $this->args['page'] = $page;
    }

    $resource = clients_resource_load($view->base_table);
    // override $resource with $this->args

    // pass params back to resource
    $args = array();
    if ($view->args[0]) { 
      $args['argument'] = $view->args[0];
    }
    if ($view->pager['items_per_page']) { 
      $args['limit'] = $view->pager['items_per_page'];
    }
    if ($view->pager['offset']) { 
      $args['offset'] = $view->pager['offset'];
    }
    clients_setparams($resource, $args);
    
    $result = clients_call($resource);

    $view->result = $result[0]->data;
    
    /**
     * Removed a bunch of pager code here - may need to go back in though should probably be handled in clients module
     */

  }
}