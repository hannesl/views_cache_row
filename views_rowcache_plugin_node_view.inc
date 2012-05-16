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

  function option_definition() {
    $options = parent::option_definition();
    $options['cache_mode'] = array('default' => 'object');
    return $options;
  }

  function cache_options() {
    return array(
      'object' => t('Node objects'),
      'rendered' => t('Rendered nodes')
    );
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['cache_mode'] = array(
      '#type' => 'radios',
      '#default_value' => $this->options['cache_mode'],
      '#title' => t('Cache mode'),
      '#options' => $this->cache_options(),
      '#description' => t('The <em>Node objects</em> mode caches the individual node objects after they have loaded, while <em>Rendered nodes</em> caches the fully rendered markup of each individual node. <em>Rendered nodes</em> may give a better performance boost, but can also result in problems with customized node information such as flags. <em>Node objects</em> is safer but may not give a noticable performance gain.'),
    );
  }

  function options_submit(&$form, &$form_state) {
    // Clear all cache items belonging to this view to ensure correct cache format.
    cache_clear_all($this->view->name . ':', 'cache_views_rowcache', true);
  }

  function summary_title() {
    $text = parent::summary_title();
    $options = $this->cache_options();
    return $text . ', ' . t('cache mode %mode', array('%mode' => $options[$this->options['cache_mode']]));
  }

  function pre_render($values) {
    $nids_load = array();
    $nodes_cached = array();
    $this->nodes = array();
    $this->nodes_rendered = array();

    foreach ($values as $row) {
      $nid = $row->{$this->field_alias};
      $cid = $this->get_cid($nid);

      // Look for a cached row.
      if ($cache = cache_get($cid, 'cache_views_rowcache')) {
        $nodes_cached[$nid] = $cache->data;
      }
      else {
        $nids_load[] = $nid;
      }
    }

    // Load node objects and cache them if using the 'object' cache mode.
    $nodes = node_load_multiple($nids_load);
    if ($this->options['cache_mode'] == 'object') {
      foreach ($nodes as $nid => $node) {
        cache_set($this->get_cid($nid), $node, 'cache_views_rowcache');
      }
      // Store node objects to be rendered in render().
      $this->nodes = $nodes_cached + $nodes;
    }
    else {
      // Store node objects to be rendered in render().
      $this->nodes = $nodes;
      // Store rendered nodes from the cache.
      $this->nodes_rendered = $nodes_cached;
    }
  }

  function render($row) {
    $nid = $row->{$this->field_alias};

    // Serve cached node content if found.
    if ($this->options['cache_mode'] == 'rendered' && isset($this->nodes_rendered[$nid])) {
      return $this->nodes_rendered[$nid];
    }

    // Render the node if needed.
    else {
      $node = $this->nodes[$nid];
      $node->view = $this->view;
      $build = node_view($node, $this->options['view_mode']);
      $output = drupal_render($build);

      // Cache rendered node if using the 'rendered' cache mode.
      if ($this->options['cache_mode'] == 'rendered') {
        // Cache the rendered node.
        cache_set($this->get_cid($nid), $output, 'cache_views_rowcache');
      }
      return $output;
    }
  }

  /**
   * Return a unique cache id for a particular node id.
   */
  function get_cid($nid) {
    return $this->view->name . ':' . $this->display->id . ':' . $nid;
  }
}
