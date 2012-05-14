<?php
/**
 * @file
 * Contains the cached node view row style plugin.
 */

/**
 * Row style plugin that inherits from the standard node style and adds row 
 * level caching.
 *
 * @ingroup views_row_plugins
 */
class views_rowcache_plugin_node_view extends views_plugin_row_node_view {

  function pre_render($values) {
    $nids_load = array();
    $nids_cached = array();
    foreach ($values as $row) {
      $nid = $row->{$this->field_alias};
      $cid = $this->get_cid($nid);

      // Look for a cached row.
      if ($cache = cache_get($cid, 'cache_views_rowcache')) {
        $nids_cached[$nid] = $cache->data;
      }
      else {
        $nids_load[] = $nid;
      }
    }

    // Store node objects to be rendered.
    $this->nodes = node_load_multiple($nids_load);
    // Store rendered nodes from the cache.
    $this->cached_nodes = $nids_cached;
  }

  function render($row) {
    $nid = $row->{$this->field_alias};

    // Serve cached node content if found.
    if (isset($this->cached_nodes[$nid])) {
      return $this->cached_nodes[$nid];
    }

    // Render the node if needed.
    else {
      $node = $this->nodes[$nid];
      $node->view = $this->view;
      $build = node_view($node, $this->options['view_mode']);
      $return = drupal_render($build);

      // Cache the rendered node.
      cache_set($this->get_cid($nid), $return, 'cache_views_rowcache');
      return $return;
    }
  }

  /**
   * Return a unique cache id for a particular node id.
   */
  function get_cid($nid) {
    return $this->view->name . ':' . $this->display->id . ':' . $nid;
  }
}
