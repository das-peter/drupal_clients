<?php
// $Id$
/**
 * @file
 * Base class for backends. Handles XML-RPC, REST calls and caching results
 *
 * @author Django Beatty - adub
 */

/**
 *
 */
abstract class ClientsServicesBase {

    /**
     * Takes variable number of params after cacheid. 
     */
    static protected function doCall($method, $cacheid) {

        $args = func_get_args();
        $args = array_slice($args, 2); // any extra params passed to this argument

        $cache_table = 'cache_clients';
        $cache_time = variable_get('clients_api_cache_time', '0');
        
        if ($cache_time == '0' || !$result = cache_get($cacheid, $cache_table)) {

            /**
             * @todo non-XMLRPC methods
             */ 
            if ($method == 'xmlrpc') {
                $data = call_user_func_array('xmlrpc', $args); 
            }
            elseif ($method == 'rest') {
                $data = call_user_func_array('drupal_http_request', $args); 
            }
            else{
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
     * Connection extending classes must implement the interface below:
     */
     
    /**
     * Sets cacheid and parameters for the service/method and calls self::doCall() 
     */
    abstract static public function call($serviceConfig, $serviceOptions);
}