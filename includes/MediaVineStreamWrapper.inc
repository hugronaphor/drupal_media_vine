<?php

/**
 *  @file
 *  Extends the MediaReadOnlyStreamWrapper class for Vine.
 */

/**
 *  Create an instance like this:
 *  $vine = new MediaVineStreamWrapper('vine://v/[video-id]');
 */
class MediaVineStreamWrapper extends MediaReadOnlyStreamWrapper {
  protected $base_url = 'http://vine.com';

  static function getMimeType($uri, $mapping = NULL) {
    return 'video/vine';
  }

  function interpolateUrl() {
    if ($params = $this->get_parameters()) {
      return $this->base_url . '/' . $params['v'];
    }
  }

  function getOriginalThumbnailPath() {
    $parts = $this->get_parameters();    
    $oembed_url = url('https://api.vineapp.com/timelines/posts/s/'. check_plain($parts['v']));    
    $response = drupal_http_request($oembed_url);

    if (!isset($response->error)) {
      $data = drupal_json_decode($response->data);
      return $data['data']['records'][0]['thumbnailUrl'];
    }
    else {
      throw new Exception("Error Processing Request. (Error: {$response->code}, {$response->error})");
      return;
    }
  }

  function getLocalThumbnailPath() {
    $parts = $this->get_parameters();
    // There's no need to hide thumbnails, always use the public system rather
    // than file_default_scheme().
    $local_path = 'public://media-vine/' . check_plain($parts['v']) . '.jpg';

    if (!file_exists($local_path)) {
      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      $response = drupal_http_request($this->getOriginalThumbnailPath());

      if (!isset($response->error)) {
        file_unmanaged_save_data($response->data, $local_path, TRUE);
      }
      else {
        @copy($this->getOriginalThumbnailPath(), $local_path);
      }
    }

    return $local_path;
  }
}
