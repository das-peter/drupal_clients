<?php
// $Id$
/**
 * @file
 * Defines methods and calls to Drupal services
 *
 * @author Django Beatty - adub
 */

/**
 *
 */
class ClientsServicesDrupal extends ClientsServicesBase {

  /**
   * Use for testing
   */
  static public function connect($connection) {
    $session = xmlrpc($connection->endpoint, 'system.connect');
    if ($session === FALSE) {

      return xmlrpc_error(); // null for services 2...
    }
    return $session;
  }

  /**
   * Prepares a hashed token for the service, based on current time, 
   * the required service and config values; serviceKey and serviceDomain
   *t
   * @param $connection
   *   stdClass: A service connection as returned by Clients::load()
   * @param $serviceMethod
   *   string: Name of service method to access
   *
   * @return
   *   array a valid token
   */
  static public function getToken($connection, $serviceMethod) {
    $timestamp = (string)time();
    $nonce = uniqid();
    $hashParameters = array($timestamp, $connection->configuration['domain'], $nonce, $serviceMethod);
    $hash = hash_hmac("sha256", implode(';', $hashParameters), $connection->configuration['servicekey']);

    return array(
      'hash'      => $hash,
      'domain'    => $connection->configuration['domain'],
      'timestamp' => $timestamp,
      'nonce'     => $nonce
    );
  }

  /**
   * Connects to Drupal Services and logs in the user provided in the config.
   * Returns a session for the user.
   * @todo needs error catching in case service is down
   *
   * @return
   *   array
   */
  static public function getUser($connection) {
    $session = self::connect($connection);

    if ($session->is_error == TRUE) {
      drupal_set_message('There was an error connecting to the service.');
      return;
    }

    $userToken = self::getToken($connection, 'user.login');

    $user = xmlrpc(
      $connection->endpoint,
      'user.login',
      $userToken['hash'],
      $userToken['domain'],
      $userToken['timestamp'],
      $userToken['nonce'],
      $session['sessid'],
      $connection->configuration['username'],
      $connection->configuration['password']
    );

    if ($user === FALSE) {
      return xmlrpc_error();
    }
    return $user;
  }

  /**
   * Gets raw data from service call
   */
  static protected function fetch($connection, $resource, $user) {
      $cacheid = md5($connection->name . implode($resource->configuration['options']));
      // user is stdClass if xmlrpc_error()...
      if (!is_array($user) || !isset($user['sessid'])) {
        // @todo watchdog
        drupal_set_message($user->message, 'error');
        return;
      }
      $token = self::getToken($connection, $resource->configuration['options']['method']);
      
      // catch empty arguments
      $arguments = array_values($resource->configuration['options']['arguments']);
      $arguments = trim($arguments[0]) != '' ? $arguments : array();
      
      $result = parent::doCall(
        'xmlrpc',
        $cacheid,
        $connection->endpoint,
        $resource->configuration['options']['method'],
        $token['hash'],
        $token['domain'],
        $token['timestamp'],
        $token['nonce'],
        $user['sessid'],
        $resource->configuration['options']['view'],
        NULL, // services 2 beta1 display id - default
//        array(), // get all fields/default display - this will work for services pre/post 2beta1
        $arguments,
        (int)$resource->configuration['options']['offset'],
        (int)$resource->configuration['options']['limit']
      );
      
      return $result;
  }

  /**
   * Executes call and processes data
   */
  static public function call($connection, $resource) {
    if ($resource->configuration['options']['method'] == 'views.get') {

      $user = self::getUser($connection);
      // gets raw result
      $result = self::fetch($connection, $resource, $user);
      // needs some post-processing
      $processed_result = new stdClass;
      $processed_result->created = $result->created;
      $processed_result->data = array();
      
      foreach ($result->data as $item) {
        
        foreach ($item as $k => $v) { // handle CCK field data structure
          if (strpos($k, 'field_') !== FALSE) {
            $item[$k] = $v[0]['value'];        
          } 
        }
        
        // nid will interfere with local nids in client
        $item['remote_nid'] = $item['nid'];
        unset($item['nid']);
        // remote taxonomy is not understood locally so flatten to RSS-style bag of tags (TODO: develop this to preserve vocabs)
        $tags = array();
        if (isset($item['taxonomy'])) {
          foreach ((array)$item['taxonomy'] as $term) { 
              $tags = $term['name'];
          }
          unset($item['taxonomy']);
        }
        $item['tags'] = $tags;
        
        $processed_result->data[] = $item;
      }
      return $processed_result;
    } // else method not supported yet
    
  }
}