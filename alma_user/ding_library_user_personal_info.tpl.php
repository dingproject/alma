<?php
/**
 * @file
 * Template for rendering a user's library user personal information.
 */
?>
<div class="vcard">
  <h4><?php print t('Basic information'); ?></h4>
  <dl>
    <dt><?php print t('Name'); ?></dt>
    <dd class="fn"><?php print check_plain($account->display_name); ?></dd>
    <dt><?php print t('Address'); ?></dt>
    <dd class="adr">
      <div class="street-address"><?php print $street; ?></div>
      <span class="postal-code"><?php print $postal_code; ?></span>
      <span class="locality"><?php print $addresses; ?></span>
    </dd>
  </dl>
  <h4><?php print t('Contact information'); ?></h4>
  <dl class="tel">
    <dt class="type"><?php print t('Mobile phone'); ?><dt>
    <dd class="value"><?php print $phone; ?></dd>
  </dl>
  <dl>
    <dt><?php print t('E-mail address'); ?></dt>
    <dd class="email"><?php print check_plain($account->mail); ?></dd>
    <dd><?php print $links; ?></dd>
  </dl>
</div>

