
-- SUMMARY --

Webform Remote Post works along with the Webform module to ease the integration
between Webforms and other web applications by extending the out-of-the-box
functionality if the Webform module.

Webform Remote Post works by POSTing form submissions to any arbitrary URL,
presumably an application or script that will use the form data and perform
further processing of it. It respects the form's validation and will only send
submissions that passed validation and are no longer in a draft state. Multiple
remote posts can be setup for each individual form, allowing for the submission
of data to multiple systems at once.

-- REQUIREMENTS --

Webform module: http://drupal.org/project/webform

The module was developed against the 6.x-3.19 version fo Webform, but it should
work with previous versions as long as they include the
hook_webform_submission_insert() hook definition.


-- INSTALLATION --

* Install as usual, see
  https://drupal.org/documentation/install/modules-themes/modules-5-6.


-- CONFIGURATION --

* Configure user permissions in Administer » Permissions:

  - admin webform remote posts

    Users in roles with the "admin webform remote posts" permission will be able
    to view the "Remote Posts" page of a webform where they can add, remove, and
    disable remote post targets.

  Note that sending data to remote targets is not affected by this permission.
  Submitting a valid form is sufficient to trigger the remote form
  submission.

* Customize the remote posts settings on on a per-webform basis:
  1) Edit the desired webform.
  2) Webform » Remote Posts
  3) Label: It's just an admin-friendly internal name for the remote post
     you are setting up. Things like "My CRM" or "My Cool App".
  4) Target URL: The full URL to POST the complete form data into. The
     receiving end should expect data as if a simple HTML form was configured
     with an "action" value of this URL.

-- TROUBLESHOOTING --

* If the target system does not seem to get form submissions:

  - Is the remote post target enabled? (Check the "Enabled" check box)
  - Is the URL correct? (Check /admin/reports/dblog to see if the post
    was rejected by the target system)
  - Try setting up a Remote Post that points to a free HTTP POST tester
    like the excellent http://postcatcher.in/
  - If you are integrating with systems like Salesforce or Eloqua, make
    sure that your form has all the needed hidden fields setup for those
    system to correctly understand who is sending them the data.

-- CONTACT --

Current maintainer:

* Enrique Delgado (enrique.delgado) - http://drupal.org/user/1080610
