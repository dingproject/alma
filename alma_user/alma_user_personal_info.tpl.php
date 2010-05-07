<?php
// $Id$

/**
 * @file alma_user_personal_info.tpl.php
 * Template for rendering a user's library user profile.
 */
?>
<div class="vcard">
  <h4><?php print t('Basic information'); ?></h4>
  <dl>
    <dt><?php print t('Name'); ?></dt>
    <dd class="fn"><?php print check_plain($alma_data['patron_name']); ?></dd>
    <dt><?php print t('Address'); ?></dt>
    <dd class="adr">
      <div class="street-address"><?php print check_plain($alma_data['addresses'][0]['street']); ?></div>
      <span class="postal-code"><?php print check_plain($alma_data['addresses'][0]['postal_code']); ?></span>
      <span class="locality"><?php print check_plain($alma_data['addresses'][0]['city']); ?></span>
    </dd>
  </dl>
  <h4><?php print t('Contact information'); ?></h4>
  <dl class="tel">
    <dt class="type"><?php print t('Mobile phone'); ?><dt>
    <dd class="value"><?php print check_plain($alma_data['phones'][0]['phone']); ?></dd>
  </dl>
  <dl>
    <dt><?php print t('E-mail address'); ?></dt>
    <dd class="email"><?php print check_plain($alma_data['mails'][0]['mail']); ?></dd>
    <dd><?php print $links; ?></dd>
  </dl>
</div>    

