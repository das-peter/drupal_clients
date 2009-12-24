<?php
// $Id$

/**
 * Class definition for Common Syndication Parser.
 * @author Django Beatty - adub
 *
 * Parses RSS and Atom feeds.
 */
class FeedsClientsParser extends FeedsParser {

  /**
   * Parses a raw string and returns a Feed object from it.
   */
  public function parse(FeedsFetcherResult $fetcherResult, FeedsSource $source) {
    // needs more work on content
    return new FeedsParserResult($fetcherResult->value);
    
  }

  /**
   * Return mapping sources.
   *
   * At a future point, we could expose data type information here,
   * storage systems like Data module could use this information to store
   * parsed data automatically in fields with a correct field type.
   */
  public function getMappingSources() {
    return clients_fields();
  }
}