#!/usr/bin/env python3

import unittest
import pyrogue.entity_db


class TestEntityDb(unittest.TestCase):
    def get_entity_db(self) -> pyrogue.entity_db.EntityDb:
        return pyrogue.entity_db.EntityDb({"entity_templates": []})

    def test_empty(self) -> None:
        entity_db = self.get_entity_db()
        self.assertEqual(entity_db.get_entity_names(), [])

    def test_create_new_entity(self) -> None:
        entity_db = self.get_entity_db()
        entity_db.create_new_entity("foo")
        self.assertEqual(entity_db.get_entity_names(), ["foo"])

    def test_resolve_inheritance_cycle(self) -> None:
        entity_db = self.get_entity_db()
        bar = entity_db.create_new_entity("bar")
        bar["from_template"] = "bar"
        with self.assertRaises(ValueError):
            entity_db.resolve_inheritance()

    def test_inheritance(self) -> None:
        entity_db = self.get_entity_db()
        foo = entity_db.create_new_entity("foo")
        foo["assemblers"] = {
            "asdf": True,
            "gzxcv": True,
            "xyz": {"1": "42", "2": "43"},
        }
        bar = entity_db.create_new_entity("bar")
        bar["from_template"] = "foo"
        bar["assemblers"] = {
            "asdf": False,
            "xyz": {"2": "44"},
        }
        entity_db.resolve_inheritance()
        self.assertEqual(
            entity_db.get_entity_by_name("bar"),
            {
                "name": "bar",
                "from_template": "foo",
                "assemblers": {
                    "asdf": False,
                    "xyz": {"1": "42", "2": "44"},
                },
            },
        )

        entity_db.compress_inheritance()
        self.assertEqual(
            entity_db.get_entity_by_name("bar"),
            {
                "name": "bar",
                "from_template": "foo",
                "assemblers": {
                    "asdf": False,
                    "xyz": {"2": "44"},
                },
            },
        )

    def test_multi_level_inheritance(self) -> None:
        entity_db = self.get_entity_db()
        foo = entity_db.create_new_entity("foo")
        foo["assemblers"] = {
            "asdf": True,
            "gzxcv": True,
            "xyz": {"1": "42", "2": "43"},
        }
        bar = entity_db.create_new_entity("bar")
        bar["from_template"] = "foo"
        bar["assemblers"] = {
            "kkk": "kkk",
            "asdf": False,
            "xyz": {"2": "44"},
        }
        baz = entity_db.create_new_entity("baz")
        baz["from_template"] = "bar"
        baz["assemblers"] = {
            "gzxcv": False,
            "xyz": {"1": "45"},
        }
        entity_db.resolve_inheritance()
        self.assertEqual(
            entity_db.get_entity_by_name("baz"),
            {
                "name": "baz",
                "from_template": "bar",
                "assemblers": {
                    "gzxcv": False,
                    "xyz": {"1": "45", "2": "44"},
                },
            },
        )

        fully_resolved_baz = entity_db.get_fully_defined_entity(
            entity_db.get_entity_names().index("baz")
        )
        self.assertEqual(
            fully_resolved_baz,
            {
                "name": "baz",
                "from_template": "bar",
                "assemblers": {
                    "asdf": False,
                    "gzxcv": False,
                    "kkk": "kkk",
                    "xyz": {"1": "45", "2": "44"},
                },
            },
        )

        entity_db.compress_inheritance()
        self.assertEqual(
            entity_db.get_entity_by_name("baz"),
            {
                "name": "baz",
                "from_template": "bar",
                "assemblers": {
                    "gzxcv": False,
                    "xyz": {"1": "45"},
                },
            },
        )


if __name__ == "__main__":
    unittest.main()
