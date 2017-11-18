# MediaInfoRenamer
A small Qt tool which analyses a file through mediainfo and renames the file by adding some of the output to the file name. (Qt runtime and mediainfo are both required for this. If you use Hybrid you could simply copy the file into the Hybrid folder.)

`MediaInfoRenamer --Inform=<InformCall>  --Separator=<used seperator> --Merger=<append> <File>`
***Options:***
* **--Inform**: 'Inform'-parameters for MediaInfo. Call "MediaInfo --Info-parameters" to get a full list of supported options.
* **--Separator**: The separator used inside the 'Inform'-parameter.
* **--Merger**: The text that should be used to combine the collected data. 
* **File:**: The file which should be analyzed and renamed.

For better understanding of what I mean with this.
*Here a few examples:*

***Example 1:***
`MediaInfoRenamer  --Inform="Video;%Width%,%Height%,%BitRate%,%FrameRate%" --Separator=","--Merger=" " "z:\Output\myfile.mkv"`
The tool then would:
a. call:
`mediainfo --Inform="Video;%Width%,%Height%,%BitRate%,%FrameRate%" "z:\Output\myfile.mkv"`
and collect the output.
`640,352,722134,25.000`

b. split the output using the separator:
* 640
* 352
* 722134
* 25.000

c. join that data using the merger:
`640 352 722134 25.000`

d. rename the file from:
`"z:\Output\myfile.mkv"`
to
`"z:\Output\myfile 640 352 722134 25.000.mkv"`

***Example 2:***
`MediaInfoRenamer  --Inform="Video;%Width/String%#%Height/String%#%BitRate/String%#%FrameRate/String%" --Separator="#" --Merger="_" "z:\Output\myfile.mkv"`
The tool then would:
a. call:
`mediainfo --Inform="Video;%Width/String%#%Height/String%#%BitRate/String%#%FrameRate/String%" "z:\Output\myfile.mkv"`
and collect the output.
`640 pixels#352 pixels#722 kb/s#25.000 FPS`

b. split the output using the separator:
* 640 pixels
* 352 pixels
* 722 kb/s
* 25.000 FPS

c. join that data using the merger:
`640 pixels_352 pixels_722 kb/s_25.000 FPS`

d. try to rename the file from:
`"z:\Output\myfile.mkv"`
to
`"z:\Output\myfile_640 pixels_352 pixels_722 kb/s_25.000 FPS.mkv"`
and fail since '/' isn't allowed inside a file name. :)

Note: The following character are reserved and thus will cause problems:
* : (colon)
* " (double quote)
* / (forward slash)
* \ (backslash)
* | (vertical bar or pipe)
* ? (question mark)
* * (asterisk)

***Example 3:***
`MediaInfoRenamer  --Inform="Video;Width %Width/String%#Height %Height/String%#%Bitrate BitRate/String%#Framerate %FrameRate/String%" --Separator="#" --Merger="_" "z:\Output\myfile.mkv"`
The tool then would:
a. call:
`mediainfo --Inform="Video;%Width/String%#%Height/String%#Bitrate(kBit pro sec) %BitRate%#Framerate(fps) %FrameRate%" "z:\Output\myfile.mkv"`
and collect the output.
`Width 640 pixels#Height 352 pixels#Bitrate(kBit pro sec) 722#Framerate(fps) 25.000`

b. split the output using the separator:
* Width 640 pixels
* Height 352 pixels
* Bitrate(kBit pro sec) 722
* Framerate 25.000 FPS

c. join that data using the merger:
`Width 640 pixels_Height 352 pixels_Bitrate 722_Framerate(fps) 25.000`

d. rename the file from:
`"z:\Output\myfile.mkv"`
to
`"z:\Output\myfile_640 pixels_352 pixels_722_Framerate(fps) 25.000.mkv"`

