TEMPLATE = subdirs

SUBDIRS += \
    PSHTest \
    PhotoscanHelper \
    PSData \
    QPLY

PSData.depends = QPLY
PhotoscanHelper.depends = PSData
PHTest.depends = PSData
