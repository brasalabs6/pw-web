#pragma once
// Checks the current WebGL2 context. Error points to a static message.
bool pw153_check_webgl_capabilities(const char **error);
// Enable S3TC on the current context when a compressed texture is required.
bool pw153_enable_s3tc();
