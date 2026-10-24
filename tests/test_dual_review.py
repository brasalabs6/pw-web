import json, pathlib, sys, tempfile, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/".agents"/"automation"))
from review_loop import guard_pr,parse_review,status,format_comment

def result(payload):
    return {"text":json.dumps(payload),"stopReason":"end_turn"}

class DualReviewContractTests(unittest.TestCase):
    def test_guard_same_repo_agent_branch_and_sha(self):
        sha="a"*40
        pr={"number":13,"draft":False,"state":"open","base":{"ref":"main"},
            "head":{"ref":"agent/e01-agent-orchestration-v1/wo-01","sha":sha,
                    "repo":{"full_name":"brasalabs6/pw-web"}}}
        self.assertEqual(guard_pr(pr,sha,"brasalabs6/pw-web",2),
                         ("e01-agent-orchestration-v1","wo-01"))
        for key,value in (("draft",True),("state","closed")):
            modified={**pr,key:value}
            with self.assertRaises(ValueError):guard_pr(modified,sha,"brasalabs6/pw-web",2)
        with self.assertRaises(ValueError):guard_pr(pr,"b"*40,"brasalabs6/pw-web",2)

    def test_parse_two_review_reports(self):
        finding={"severity":"high","path":"engine/core/pw_core.cpp","line":7,
                 "title":"Unchecked allocation","detail":"No upper size limit",
                 "recommendation":"Reject oversized input"}
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"review.json"
            payload={"decision":"changes_requested","summary":"One regression",
                     "findings":[finding],"tests_assessed":["C++ smoke"],"residual_risks":[]}
            p.write_text(json.dumps(result(payload)))
            parsed=parse_review(p,["engine/core/pw_core.cpp"])
            self.assertEqual(parsed["findings"][0]["severity"],"high")
            self.assertEqual(status({"deepseek":parsed,"glm-flash":parsed},"success",1,True,8),"repair")
            self.assertEqual(status({"deepseek":parsed,"glm-flash":parsed},"success",8,True,8),"blocked")
            payload["decision"]="approved"
            p.write_text(json.dumps(result(payload)))
            with self.assertRaises(ValueError):parse_review(p,["engine/core/pw_core.cpp"])
            payload["decision"]="changes_requested"
            payload["findings"][0]["path"]=".github/workflows/ci.yml"
            p.write_text(json.dumps(result(payload)))
            with self.assertRaises(ValueError):parse_review(p,["engine/core/pw_core.cpp"])

    def test_approval_requires_both_and_green_ci(self):
        approval={"decision":"approved","summary":"Reviewed","findings":[]}
        pair={"deepseek":approval,"glm-flash":approval}
        self.assertEqual(status(pair,"success",3,True,8),"human-review")
        self.assertEqual(status(pair,"failure",3,True,8),"repair")
        self.assertEqual(status({"deepseek":approval},"success",3,True,8),"blocked")
        comment=format_comment(pair,"a"*40,3,"success","human-review","100")
        self.assertIn("pw-agent-review",comment)
        self.assertIn("deepseek",comment)
        self.assertIn("glm-flash",comment)

    def test_automated_loop_opt_in_and_capped(self):
        changes={"decision":"changes_requested","summary":"Need fixes",
                 "findings":[{"severity":"medium","path":"web/app.js","line":4,
                              "title":"Missing handler","detail":"Observed missing callback",
                              "recommendation":"Add regression test"}]}
        pair={"deepseek":changes,"glm-flash":changes}
        self.assertEqual(status(pair,"success",1,False,20),"blocked")
        self.assertEqual(status(pair,"success",19,True,20),"repair")
        self.assertEqual(status(pair,"success",20,True,20),"blocked")

if __name__=="__main__":
    unittest.main()
