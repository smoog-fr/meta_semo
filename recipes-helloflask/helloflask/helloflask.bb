# Unable to find any files that looked like license statements. Check the accompanying
# documentation and source headers and set LICENSE and LIC_FILES_CHKSUM accordingly.
#

DESCRIPTION = "Simple helloflask application"
SECTION = "examples"
DEPENDS = ""

# NOTE: LICENSE is being set to "CLOSED" to allow you to at least start building - if
# this is not accurate with respect to the licensing of the software being built (it
# will not be in most cases) you must specify the correct value before using this
# recipe for anything other than initial testing/development!
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

# add Files from git
SRC_URI[md5sum] = "8849646e6e5d2842ae3e1b63c4769159"
SRC_URI[sha256sum] = "56b9dfab9999cd49fa6b789a30b287659e994a39f9d8f2767622801dbd136da3"
SRC_URI = "git://github.com/zgdavid/helloflask.git;branch=main \
           file://zadmin.service \
           "

# Commit Number from Git-Repo
SRCREV = "a5ae930411c6a680e49811eeba36d702d8ad47d1"

S = "${WORKDIR}/git"

# sources from anywhere of the desktop
inherit externalsrc
#EXTERNALSRC = "/home/david/Desktop/helloflask"
#EXTERNALSRC_BUILD = "/home/david/Desktop/helloflask"

# setuptools for python
inherit setuptools3

# systemd for autostart service
inherit systemd
SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE_${PN} = "zadmin.service"

# add to Image (rootfs?)
FILES_${PN} += "${systemd_system_unitdir}"

do_configure () {
	# Specify any needed configure commands here
	:
}

do_compile () {
	# Specify compilation commands here
	:
}

do_install () {
	install -d ${D}${bindir}
	install -d ${D}${bindir}/zadmin
        install -m 0755 ${S}/catseverywhere.py ${D}${bindir}/zadmin
	install -d ${D}${bindir}/zadmin/templates
	install -m 0755 templates/layout.html ${D}${bindir}/zadmin/templates
	install -m 0755 templates/home.html ${D}${bindir}/zadmin/templates
	install -m 0755 templates/about.html ${D}${bindir}/zadmin/templates
	install -d ${D}${bindir}/zadmin/static/css
	install -m 0755 static/css/main.css ${D}${bindir}/zadmin/static/css

	install -d ${D}${systemd_system_unitdir}
	install -m 0644 ${WORKDIR}/zadmin.service ${D}${systemd_system_unitdir}
}

REQUIRED_DISTRO_FEATURES= "systemd"
