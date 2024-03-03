.PHONY: test-all
test-all:
	python -m pytest -s --verbose test_setup.py --restart=False --clean=False
	python -m pytest -s --verbose test_control.py --restart=False --token_key=custom_token

.PHONY: test-setup-clean
test-setup-clean:
	python -m pytest -s --verbose  test_setup.py --restart=True --clean=True

.PHONY: test-control-android
test-control-android:
	python -m pytest -s --verbose test_android.py --delay_sec=0
	python -m pytest -s --verbose test_control.py --delay_sec=0 --token_key=android_token


