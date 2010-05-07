/**
 * Drupal behaviour to use a jQuery UI dialog for login links.
 */
Drupal.behaviors.almaUserLoginDialog = function () {
    var loginPath = Drupal.settings.basePath + 'user/login';
    $("a[href^='" + loginPath + "']").click(function () {
        $('<div></div>').html($('#user-login-form').clone()).dialog({
            'close': function (event, ui) {
                $(this).dialog('destroy').remove();
            },
            'modal': true,
            'title': Drupal.t('User login')
        });
        return false;
    });
};

