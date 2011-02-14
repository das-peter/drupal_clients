<?php
// $Id$
/**
 * @file
 * Defines Flickr services and calls
 *
 * @author Django Beatty - adub
 */

/**
 *
 */
class clients_connection_flickr extends clients_connection_base {

  /**
   * Use for testing
   */
  static public function connect($connection) {
    $session = drupal_http_request($connection->endpoint .'?method=flickr.test.echo');
    if ($session === FALSE) {
      drupal_set_message('Error connecting');
    }
    return $session;
  }

  /**
   * Prepares a hashed token for the service, based on current time, 
   * the required service and config values; serviceKey and serviceDomain
   *
   * @param $serviceConfig
   *   stdClass: A service configuration as returned by Clients::load()
   * @param $serviceMethod
   *   string: Name of service method to access
   * 
   * @return
   *   array: a valid token
   */
  static public function getToken($connection, $serviceMethod) {
    // @todo - http://www.flickr.com/services/api/auth.howto.web.html
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
    $userToken = self::getToken($connection, 'user.login');
    
    $user = xmlrpc(
      $connection->baseurl,
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
   * Executes call and processes data
   */
  static public function call($serviceConfig, $serviceOptions) {
    
    if ($serviceOptions->configuration['options']['method'] == 'flickr.photos.search') {
      // gets raw result
      $cache_id = md5($serviceConfig->name . implode($serviceOptions->configuration['options']));
      $fetch_url = $serviceConfig->endpoint 
        .'?method='. $serviceOptions->configuration['options']['method']
        .'&api_key='. $serviceConfig->configuration['api_key']
        .'&text='. $serviceOptions->configuration['options']['text']
        .'&page='. $serviceOptions->configuration['options']['page']
        .'&per_page='. $serviceOptions->configuration['options']['per_page']; 
        // etc http://www.flickr.com/services/api/flickr.photos.search.html
        
      $result = parent::doCall(
        'rest',
        $cache_id,
        $fetch_url 
      );
      return $result;
    } // else method not supported yet
  }
}