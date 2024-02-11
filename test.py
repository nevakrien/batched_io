from c_batched_IO import write_json_to_file
import json
import time
import tempfile
import os

def compare_serialization_speed_and_output(data):
    # Measure time and write using the standard json package
    start_time = time.time()
    with tempfile.NamedTemporaryFile(delete=False, mode='w+', suffix='.json') as tmpfile:
        json_temp_filename = tmpfile.name
        json.dump(data, tmpfile)
    json_duration = time.time() - start_time

    # Read back the content written by json.dump
    with open(json_temp_filename, 'r') as f:
        json_output = json.load(f)

    # Measure time for your c_batched_IO package
    start_time = time.time()
    with tempfile.NamedTemporaryFile(delete=False, mode='w+', suffix='.json') as tmpfile:
        c_batched_IO_temp_filename = tmpfile.name
    write_json_to_file(data, c_batched_IO_temp_filename)
    c_batched_IO_duration = time.time() - start_time

    # Read back the content written by write_json_to_file
    with open(c_batched_IO_temp_filename, 'r') as f:
        try:
        	s=f.read()
        	c_batched_IO_output = json.loads(s)
        except Exception as e:
        	print(s)
        	raise e
    # Compare the outputs
    is_output_same = json_output == c_batched_IO_output

    # Cleanup temp files
    os.remove(json_temp_filename)
    os.remove(c_batched_IO_temp_filename)

    assert is_output_same

    return json_duration, c_batched_IO_duration, is_output_same

def ensure_crash(wrong):
	assert not isinstance(wrong,dict) #we need to pass the wrong datatype...
	assert not os.path.exists('temp') #temp file should not exist already...

	try:
		write_json_to_file(wrong,'temp')
	except Exception as e:
		if os.path.exists('temp'):
			os.remove('temp')
			raise Exception('failed to remove file')
		return

	os.remove('temp')
	raise Exception('did not fail on wrong data')


if __name__=="__main__":
	class dumb:
		def __init__(self):
			self.dumb_stuff='meh'
	ensure_crash(dumb())

	# Example usage with a test dictionary
	data = {"key": "value", "int": 1, "bool": True,"list": [1, 2, 3]}
	json_duration, c_batched_IO_duration, outputs_are_same = compare_serialization_speed_and_output(data)

	print(f"JSON package duration: {json_duration}s")
	print(f"c_batched_IO package duration: {c_batched_IO_duration}s")
	print(f"Outputs are the same: {outputs_are_same}")


	