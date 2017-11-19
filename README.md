# MediaInfoRenamer
A small Qt tool which analyses a file through mediainfo and renames the file by adding some of the output to the file name. (Qt runtime and mediainfo are both required for this. If you use Hybrid you could simply copy the file into the Hybrid folder.)

`MediaInfoRenamer --Inform=<InformCall>  --Separator=<used seperator> --Merger=<append> <File>`
***Options:***
* **--Inform**: 'Inform'-parameters for MediaInfo. Call "MediaInfo --Info-parameters" to get a full list of supported options.
* **--EncodingSettings**: 'Inform'-parameters for MediaInfo. Call "mediainfo --Info-parameters" to get a full list of supported options.
* **--Separator**: The separator used inside the 'Inform'-parameter.
* **--Merger**: The text that should be used to combine the collected data.
* **--Replacements**: List of replacements separated by 'Separator' (replace x with Y: X%Y; )
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

***Example 4:***
`MediaInfoRenamer --Separator="#" --Merger="_" --Inform="Video;Bitrate %BitRate/String%" --EncodingSettings="CRF %crf=%" --Inform="Video;Width %Width/String%#Height %Height/String%" --Replacements="kb/s%kbps" "z:\Output\myfile.mkv"`
The tool then would:
a. call:
`mediainfo --Inform=Video;Bitrate %BitRate/String%"`
and collect the output.
`Bitrate 722 kb/s`

b. apply the replacements
Replacing 'kb/s' with 'kbps' and thus getting:
`Bitrate 722 kbps`

c. remembering the addition and checking for the next parameter

d. call:
`mediainfo --Inform="Video;%Encoded_Library_Settings%" "z:\Output\myfile.mkv"`
and collect the output.

e. split the output using '/', select the wanted values ('crf=18.0'), apply the replacements.
`CRF 18.0`

f. remembering the addition and checking for the next parameter

g. call:
`mediainfo --Inform="Video;Width %Width/String%#Height %Height/String%"`
and collect the output.
`Width 604 pixel#Height 480 pixel`

f. split with the 'Separator', apply the replacements and remember the addition

c. join that additions using the 'Merger'
`Bitrate 722 kbps_CRF 18.0_Width 640 pixels_Height 352 pixels`

d. rename the file from:
`"z:\Output\myfile.mkv"`
to
`"z:\Output\myfile_Bitrate 722 kbps_CRF 18.0_Width 640 pixels_Height 352 pixels.mkv"`
