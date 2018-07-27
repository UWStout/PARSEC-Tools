TEMPLATE = subdirs

SUBDIRS += \
    PSHTest \
    PhotoscanHelper \
    PSData \
    QPLY

PSData.depends = QPly
PhotoscanHelper.depends = PSData
PHTest.depends = PSData
