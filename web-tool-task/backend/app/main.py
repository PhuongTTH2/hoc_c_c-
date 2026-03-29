from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.api import auth, tasks, dashboard
from app.core.database import engine, Base, SessionLocal
from app.core.security import get_password_hash
from app.models.user import User
from app.models.task import Task
from app.models.jira import JiraDetails
from app.models.git import GitDetails
from app.models.sharepoint import SharePointDetails
from app.models.confluence import ConfluenceDetails
import os

# Create all tables
Base.metadata.create_all(bind=engine)

# Create default admin user if not exists
def create_default_admin():
    db = SessionLocal()
    try:
        admin = db.query(User).filter(User.email == "admin@example.com").first()
        if not admin:
            admin_user = User(
                email="admin@example.com",
                username="admin",
                full_name="Administrator",
                hashed_password=get_password_hash("admin123"),
                role="ADMIN",
                is_active=True
            )
            db.add(admin_user)
            db.commit()
            print("✅ Default admin user created: admin@example.com / admin123")
        else:
            print("✅ Admin user already exists")
    except Exception as e:
        print(f"Error creating admin: {e}")
    finally:
        db.close()

create_default_admin()

app = FastAPI(title="Web Tool Automation API", version="1.0.0")

# CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000", "http://localhost:8000"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Routers
app.include_router(auth.router, prefix="/api/auth", tags=["Auth"])
app.include_router(tasks.router, prefix="/api/tasks", tags=["Tasks"])
app.include_router(dashboard.router, prefix="/api/dashboard", tags=["Dashboard"])

@app.get("/")
def root():
    return {"message": "Web Tool Automation API", "status": "running"}

@app.get("/health")
def health():
    return {"status": "healthy", "database": "connected"}