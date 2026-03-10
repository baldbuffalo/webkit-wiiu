# Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
# Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com> 
# Copyright (C) 2009 Cameron McCormack <cam@mcc.id.au>
# Copyright (C) 2012-2014 ACCESS Co., Ltd.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer. 
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution. 
# 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

VPATH = \
    $(AppSrcDir) \
#

DOM_CLASSES = \
    Wiiu \
    Remote \
    WiiuGamePad \
    VideoPlayer \
    ImageView
#

.PHONY : all

JS_DOM_HEADERS=$(filter-out JSMediaQueryListListener.h JSEventListener.h JSEventTarget.h,$(DOM_CLASSES:%=JS%.h))

all : \
    $(JS_DOM_HEADERS) \
#

ADDITIONAL_IDL_DEFINES :=

ifndef ENABLE_DASHBOARD_SUPPORT
    ENABLE_DASHBOARD_SUPPORT = 0
endif

ifndef ENABLE_ORIENTATION_EVENTS
    ENABLE_ORIENTATION_EVENTS = 0
endif

ifeq ($(ENABLE_ORIENTATION_EVENTS), 1)
    ADDITIONAL_IDL_DEFINES := $(ADDITIONAL_IDL_DEFINES) ENABLE_ORIENTATION_EVENTS
endif

# --------

# Common generator things

GENERATE_SCRIPTS = \
    $(WebCore)/bindings/scripts/CodeGenerator.pm \
    $(WebCore)/bindings/scripts/IDLParser.pm \
    $(WebCore)/bindings/scripts/IDLStructure.pm \
    $(WebCore)/bindings/scripts/CodeGeneratorJS.pm \
    $(WebCore)/bindings/scripts/generate-bindings.pl

generator_script = perl -I $(WebCore)/bindings/scripts/ $(WebCore)/bindings/scripts/generate-bindings.pl

# JS bindings generator

IDL_INCLUDES = \
    $(WebCore)/dom \
    $(WebCore)/fileapi \
    $(WebCore)/html \
    $(WebCore)/css \
    $(WebCore)/page \
    $(WebCore)/notifications \
    $(WebCore)/xml \
    $(WebCore)/svg \
    $(AppSrcDir)

IDL_COMMON_ARGS = $(IDL_INCLUDES:%=--include %) --write-dependencies --outputDir .

JS_BINDINGS_SCRIPTS = $(GENERATE_SCRIPTS)

JS%.h : %.idl $(JS_BINDINGS_SCRIPTS)
	$(call generator_script, $(JS_BINDINGS_SCRIPTS)) $(IDL_COMMON_ARGS) --defines "$(FEATURE_DEFINES) $(ADDITIONAL_IDL_DEFINES) LANGUAGE_JAVASCRIPT" --generator JS $<

# Inspector interfaces generator

# ------------------------

