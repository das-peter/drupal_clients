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

  // ============================================ Connection UI

  /**
   * Format the connection's endpoint as a link.
   *
   * @param $url
   *  The connection's endpoint.
   *
   * @return
   *  The string to display in the admin UI. Subclasses may format this as a
   *  link to the remote site.
   */
  function formatEndpoint($url) {
    return $url;
  }

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
   * Provide buttons for the connection testing page.
   */
  function getTestOperations() {
    return array();
  }

  // ============================================ Constructor.

  /**
   * Constructor method.
   *
   * @param $object
   *  An object of class stdClass returned from CTools.
   */
  function __construct($object) {
    // Lump all data unto the object...
    foreach ((array) $object as $field => $value) {
      $this->$field = $value;
    }

    // Connections defined in code are already unserialized.
    if (!is_array($object->configuration)) {
      $this->configuration = unserialize($object->configuration);
    }

    return $this;
  }

  // ============================================ Connection API.

  /**
   * Call a remote method.
   *
   * This is a wrapper around callMethodArray that gives the convenience of
   * being able to pass method name and parameters as one flat list, and hence
   * is the main API for connection objects.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param ...
   *  All other parameters are passed to the remote method.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethod($method) {
    // Get all the arguments this function has been passed.
    $function_args  = func_get_args();
    // Slice out the ones that are arguments to the method call: everything past
    // the 1st argument.
    $method_params  = array_slice($function_args, 1);

    return $this->callMethodArray($method, $method_params);
  }

  /**
   * Call a remote method with an array of parameters.
   *
   * This is intended for internal use from callMethod() and
   * clients_connection_call().
   * If you need to call a method on given connection object, use callMethod
   * which has a nicer form.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param $method_params
   *  An array of parameters to passed to the remote method.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethodArray($method, $method_params = array()) {
    // Up to subclasses to override this to do something.
  }
}