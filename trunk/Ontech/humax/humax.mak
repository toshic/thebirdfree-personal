
kalimba_dir := ./

# The rules here will copy any DSP images that have been built to the image directory of the application.
# If a DSP image is not to be included in the application make sure the DSP image doesn't exist,
# or the rules in this file are updated to only copy across the required DSP images.

# Any DSP images will exist in the specific directory structure.
dsp_images := $(wildcard $(kalimba_dir)/*/image/*)

# Copy any .kap or 000 files that exist:
# Copy from ../../kalimba/apps/<nameA>/image/<nameB>/<nameB>.kap to image/<nameB>/<nameB>.kap
# Copy from ../../kalimba/apps/<nameA>/image/<nameB>/000 to image/<nameB>/000
# The copykap utility takes a list of ../../kalimba/apps/<nameA>/image/<nameB> as arguments and does the rest for us!
copydsp:
	$(copykap) $(dsp_images)
	
image.fs : copydsp

clean ::
	$(del) $(wildcard image/*/*.kap) 
	$(del) $(wildcard image/*/000)
