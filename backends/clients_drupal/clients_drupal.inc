<?php
// $Id$
/**
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
  static public function connect($connection)
  {
    $session = xmlrpc($connection->endpoint, 'system.connect');
    if ($session === FALSE) {

      return xmlrpc_error(); // null for services 2...
    }
    return $session;
  }

  /**
   * Prepares a hashed token for the service, based on current time, the required service
   * and config values; serviceKey and serviceDomain
   * @param stdClass $serviceConfig A service configuration as returned by Clients::load()
   * @param string $serviceMethod Name of service method to access
   *
   * @return array a valid token
   */
  static public function getToken($connection, $serviceMethod)
  {
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
   *
   * @return array
   * @todo needs error catching in case service is down
   */
  static public function getUser($connection)
  {
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
  static protected function fetch($serviceConfig, $serviceOptions, $user) {
      $cacheid = md5($serviceConfig->name . implode($serviceOptions->configuration['options']));
      // user is stdClass if xmlrpc_error()...
      if(!is_array($user) || !isset($user['sessid'])) {
        // TODO watchdog
        drupal_set_message($user->message, 'error');
        return;
      }
      $token = self::getToken($serviceConfig, $serviceOptions->configuration['options']['method']);
      $result = parent::doCall(
        'xmlrpc',
        $cacheid,
        $serviceConfig->endpoint,
        $serviceOptions->configuration['options']['method'],
        $token['hash'],
        $token['domain'],
        $token['timestamp'],
        $token['nonce'],
        $user['sessid'],
        $serviceOptions->configuration['options']['view'],
        array(), // get all fields
        array_values($serviceOptions->configuration['options']['arguments']),
        (int)$serviceOptions->configuration['options']['offset'],
        (int)$serviceOptions->configuration['options']['limit']
      );
      return $result;
  }

  /**
   * Executes call and processes data
   */
  static public function call($serviceConfig, $serviceOptions) {
    if($serviceOptions->configuration['options']['method'] == 'views.get') {

      $user = self::getUser($serviceConfig);
      // gets raw result
      $result = self::fetch($serviceConfig, $serviceOptions, $user);
      // needs some post-processing
      $processed_result = new stdClass;
      $processed_result->created = $result->created;
      $processed_result->data = array();
      
      foreach($result->data as $item) {
        // nid will interfere with local nids in client
        $item['remote_nid'] = $item['nid'];
        unset($item['nid']);
        $tags = array();
        // remote taxonomy is not understood locally so flatten to RSS-style bag of tags (TODO: develop this to preserve vocabs)
        /* - tofix: item may not have taxo
        foreach($item['taxonomy'] as $term) {
            $tags = $term['name'];
        }
        */
        $item['tags'] = $tags;
        unset($item['taxonomy']);
        $processed_result->data[] = $item;
      }
      return $processed_result;
    } // else method not supported yet
    
  }
}