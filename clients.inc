<?php
/**
 * @file
 * Base class for backends. Handles XML-RPC, REST calls and caching results
 *
 */

/**
 * Base class for Client connections.
 */
abstract class clients_connection_base {
  // Class properties.
  public $name;
  public $cid;
  public $endpoint;
  public $configuration;

  /**
   * Submit handler for saving/updating connections of this class.
   *
   * @see clients_connection_form_submit().
   */
  static function connectionSettingsForm_submit($form, &$form_state) {
    // Base class does nothing; saving of the connection is handled by the
    // 'real' FormAPI submit handler.
  }

  /**
   * Constructor method.
   *
   * @param $row
   *  The database row containing the connection data.
   */
  function __construct($row) {
    // Lump all data unto the object...
    foreach ($row as $field => $value) {
      $this->$field = $value;
    }

    $this->configuration = unserialize($row['configuration']);

    return $this;
  }

  /**
   * Takes variable number of params after cacheid.
   */
  static protected function doCall($method, $cacheid) {

    $args = func_get_args();
    $args = array_slice($args, 2); // any extra params passed to this argument

    $cache_table = 'cache_clients';
    $cache_time = variable_get('clients_api_cache_time', '0');

    if ($cache_time == '0' || !$result = cache_get($cacheid, $cache_table)) {
      if ($method == 'xmlrpc') {
        $data = call_user_func_array('xmlrpc', $args);
      }
      elseif ($method == 'rest') {
        $data = call_user_func_array('drupal_http_request', $args);
      }
      else {
        $data = t('@method not yet supported', array('@method' => $method));
      }

      // @todo error handling/reporting
      if ($cache_time != '0') {
        cache_set(
          $cacheid,
          $data,
          $cache_table,
          $cache_time == 'cron' ? CACHE_TEMPORARY :
            time() + ((int)$cache_time * 60)
          );
      }
      $result = new stdClass;
      $result->data = $data;
      $result->created = time();
    }
    return $result;
  }

  /**
   * Provide buttons for the connection testing page.
   */
  function getTestOperations() {
    return array();
  }

  /**
   * Connection extending classes must implement the interface below:
   */

  /**
   * Sets cacheid and parameters for the service/method and calls self::doCall()
   */
  abstract static public function call($serviceConfig, $serviceOptions);
}