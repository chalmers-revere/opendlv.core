# Makefile - Makefile to build opendlv.core using Docker-encapsulated builders.
# Copyright (C) 2016 Christian Berger
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

PROJECT_NAME=opendlv.core

REPOSITORY=seresearch

IMAGE=opendlv-core-on-opendavinci-on-base
IMAGE_PATH=$(REPOSITORY)/$(IMAGE)

BASE_IMAGE=opendavinci-on-base
BASE_IMAGE_PATH=$(REPOSITORY)/$(BASE_IMAGE)
BASE_IMAGE_VERSION=v4.16.0

DEV_SUFFIX=-dev

IMAGE_DEV=$(IMAGE)$(DEV_SUFFIX)
IMAGE_DEV_PATH=$(REPOSITORY)/$(IMAGE_DEV)

BASE_IMAGE_DEV_PATH=$(REPOSITORY)/$(BASE_IMAGE)$(DEV_SUFFIX)
BASE_IMAGE_DEV_VERSION=$(BASE_IMAGE_VERSION)

DOCKER=$(shell which docker)
CMAKE=$(shell which cmake)

VERSION=v$(shell head -1 ../VERSION | cut -f1 -d" ")
GIT_CURRENT_BRANCH=$(shell git rev-parse --abbrev-ref HEAD)

SOURCE_TREE_PATH=$(shell dirname $(shell pwd))
BUILD_PATH=$(SOURCE_TREE_PATH)/docker/builds/$(IMAGE)-$(GIT_CURRENT_BRANCH)

RUN_BUILD_UID_AS=$(shell getent passwd `whoami` | cut -f3 -d":")

###############################################################################

.PHONY: all buildComplete createDockerImage createDockerImageDev createDockerImageRun clean pushDockerImage

all: buildComplete createDockerImage

clean:
	$(CMAKE) -E remove_directory $(BUILD_PATH)

buildComplete:
	$(CMAKE) -E make_directory $(BUILD_PATH)/build $(BUILD_PATH)/output
	$(CMAKE) -E copy $(SOURCE_TREE_PATH)/script/opendlv-builder.sh $(BUILD_PATH)/output
	$(DOCKER) run -ti --rm -v $(SOURCE_TREE_PATH):/opt/sources -v $(BUILD_PATH)/build:/opt/build -v $(BUILD_PATH)/output:/opt/output -v $(SOURCE_TREE_PATH)/docker/build.sh:/opt/build.sh -w /opt $(BASE_IMAGE_DEV_PATH):$(BASE_IMAGE_DEV_VERSION) /opt/build.sh $(RUN_BUILD_UID_AS)

createDockerImage: createDockerImageRun createDockerImageDev

createDockerImageRun:
	cat $(SOURCE_TREE_PATH)/docker/Dockerfile.template.run | sed -e s%BASEIMAGE%$(BASE_IMAGE_PATH)% > $(BUILD_PATH)/output/Dockerfile
	cd $(BUILD_PATH)/output && $(DOCKER) build -t $(IMAGE_PATH):latest .
	$(DOCKER) tag $(IMAGE_PATH):latest $(IMAGE_PATH):$(VERSION)

createDockerImageDev:
	cat $(SOURCE_TREE_PATH)/docker/Dockerfile.template.dev | sed -e s%BASEIMAGE%$(BASE_IMAGE_DEV_PATH)% > $(BUILD_PATH)/output/Dockerfile
	cd $(BUILD_PATH)/output && $(DOCKER) build -t $(IMAGE_DEV_PATH):latest .
	$(DOCKER) tag $(IMAGE_DEV_PATH):latest $(IMAGE_DEV_PATH):$(VERSION)

###############################################################################

pushDockerImage: createDockerImage
	$(DOCKER) push $(IMAGE_PATH):latest
	$(DOCKER) push $(IMAGE_PATH):$(VERSION)
	$(DOCKER) push $(IMAGE_DEV_PATH):latest
	$(DOCKER) push $(IMAGE_DEV_PATH):$(VERSION)
