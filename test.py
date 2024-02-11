import c_batched_IO
from c_batched_IO import write_json_to_file,batch_dump
import json
import time
import tempfile
import os

from concurrent.futures import ThreadPoolExecutor
import uuid

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


def compare_batched_serialization_speed_and_output(data_list):
    # Create a temporary directory
    temp_dir = tempfile.mkdtemp()

    # Generate unique file names within the temporary directory
    file_names = [os.path.join(temp_dir, f"{uuid.uuid4()}.json") for _ in data_list]

    # Function to serialize data using json.dump
    def py_write(data, file_name):
        with open(file_name, 'w') as f:
            json.dump(data, f)

    # Measure time for serializing with the standard json package using ThreadPoolExecutor
    start_time = time.time()
    with ThreadPoolExecutor() as executor:
        executor.map(py_write, data_list, file_names)
    json_duration = time.time() - start_time

    # Collect outputs after serialization for comparison
    json_outputs_dict = {file_name: json.load(open(file_name, 'r')) for file_name in file_names}

    # Measure time for your batched serialization function
    # Reset file names for c_batched_IO serialization to ensure a fair comparison
    start_time = time.time()
    batch_dump(data_list, file_names)
    c_batched_IO_duration = time.time() - start_time

    # Collect outputs after batch_dump serialization for comparison
    c_batched_IO_outputs_dict = {}
    for file_name in file_names:
        with open(file_name, 'r') as f:
            try:
                s = f.read()
                c_batched_IO_outputs_dict[file_name] = json.loads(s)
            except Exception as e:
                print('c wrote:')
                print(s)
                #print("Error during batch_dump serialization or reading:", e)
                raise

    # Compare the outputs
    #is_output_same = all(json_outputs_dict[fn] == c_batched_IO_outputs_dict[fn_c] for fn, fn_c in zip(sorted(file_names), sorted(file_names)))
    is_output_same= json_outputs_dict==c_batched_IO_outputs_dict 

    # Cleanup: Remove the temporary directory and its contents
    for file_name in file_names:
        os.remove(file_name)
    os.rmdir(temp_dir)

    #assert is_output_same

    return json_duration, c_batched_IO_duration, is_output_same

if __name__=="__main__":
	print(f'showing the modules stuff\n{dir(c_batched_IO)}')
	class dumb:
		def __init__(self):
			self.dumb_stuff='meh'
	ensure_crash(dumb())

	# Example usage with a test dictionary
	data = {"key": "value", "int": 1, "bool": True,"list": [1, 2, 3],'tup':(2,1)}
	json_duration, c_batched_IO_duration, outputs_are_same = compare_serialization_speed_and_output(data)

	print(f"JSON package duration: {json_duration}s")
	print(f"c_batched_IO package duration: {c_batched_IO_duration}s")
	print(f"Outputs are the same: {outputs_are_same}")


	# Example usage with a test dictionary
	data_list = [{"key": "value", "int": 1, "bool": True,"list": [1, 2, 3],'tup':(2,1)},{"bool": False,"list": [1, 2,]}] #{'key':"value"}]}]
	json_duration, c_batched_IO_duration, outputs_are_same = compare_batched_serialization_speed_and_output(data_list)

	print(f"JSON package duration: {json_duration}s")
	print(f"c_batched_IO package duration: {c_batched_IO_duration}s")
	print(f"Outputs are the same: {outputs_are_same}")